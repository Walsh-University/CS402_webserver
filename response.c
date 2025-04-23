#include "response.h"
#include "file_handler.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_handler.h"

const char *get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (ext) {
        if (strcmp(ext, ".html") == 0) return "text/html";
        if (strcmp(ext, ".css") == 0) return "text/css";
        if (strcmp(ext, ".js") == 0) return "application/javascript";
        if (strcmp(ext, ".png") == 0) return "image/png";
        if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(ext, ".txt") == 0) return "text/plain";
    }
    return "application/octet-stream";
}

// Helper function for hex conversion
int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// URL decode function to handle encoded characters in form submissions
char *url_decode(const char *src) {
    size_t src_len = strlen(src);
    char *decoded = malloc(src_len + 1);
    if (!decoded) return nullptr;

    size_t j = 0;
    for (size_t i = 0; i < src_len; i++) {
        if (src[i] == '%' && i + 2 < src_len) {
            int high = hex_to_int(src[i+1]);
            int low = hex_to_int(src[i+2]);
            if (high != -1 && low != -1) {
                decoded[j++] = (high << 4) | low;
                i += 2;
                continue;
            }
        } else if (src[i] == '+') {
            decoded[j++] = ' ';
            continue;
        }
        decoded[j++] = src[i];
    }
    decoded[j] = '\0';
    return decoded;
}

char *get_form_value(const char *body, size_t body_len, const char *key) {
    if (!body || !key) return nullptr;

    size_t key_len = strlen(key);
    const char *pos = body;
    const char *end = body + body_len;

    while (pos < end) {
        // Find the next occurrence of the key
        if (strncmp(pos, key, key_len) == 0 && pos[key_len] == '=') {
            // Found the key, extract the value
            const char *value_start = pos + key_len + 1;
            const char *value_end = strchr(value_start, '&');
            if (!value_end) value_end = end; // Last parameter

            size_t value_len = value_end - value_start;
            char *value = malloc(value_len + 1);
            if (!value) return nullptr;

            memcpy(value, value_start, value_len);
            value[value_len] = '\0';

            // URL decode the value
            char *decoded = url_decode(value);
            free(value);
            return decoded;
        }

        // Move to the next parameter
        pos = strchr(pos, '&');
        if (!pos) break;
        pos++; // Skip the '&'
    }

    return nullptr; // Key not found
}

void handle_form_post(const Request *req, Response *res) {
    // Pretend we're not processing a login form ;)
    char *username = get_form_value(req->body, strlen(req->body), "username");
    char *password = get_form_value(req->body, strlen(req->body), "password");

    if (username && password) {
        // TODO - Normally you would validate the username and password here
        //    Preferably use an identity provider like Auth0 or Okta

        //  We're just simulating some POST form data processing
        res->body = strdup("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nLogin Successful");
    } else {
        res->body = strdup("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nMissing username or password");
    }

    // Free allocated memory
    if (username) free(username);
    if (password) free(password);
}

// Function to handle POST FORM or JSON requests
void handle_post_request(const Request *req, Response *res) {
    const char *content_type = nullptr;

    // Search for the "Content-Type" header
    for (int i = 0; i < req->header_count; i++) {
        if (strcasecmp(req->header_list[i].key, "Content-Type") == 0) {
            content_type = req->header_list[i].value;
            break;
        }
    }

    if (!content_type) {
        res->body = strdup("HTTP/1.1 400 Bad Request\r\n\r\nMissing Content-Type header");
        return;
    }

    if (strstr(content_type, "form") != NULL) {
        handle_form_post(req, res);
    }
    else if (strstr(content_type, "json") != NULL) {
        handle_json_request(req);
    }
    else {
        res->body = strdup("HTTP/1.1 415 Unsupported Media Type\r\n\r\n");
        return;
    }
}

Response handle_request(const Request *req) {
    Response res = {0};

    if (strcmp(req->method, "GET") == 0) {
        char filepath[512];
        if (strcmp(req->path, "/") == 0) {
            snprintf(filepath, sizeof(filepath), "public/index.html");
        } else {
            snprintf(filepath, sizeof(filepath), "public%s", req->path);
        }
        char *file_contents = read_file(filepath);
        if (file_contents == NULL) {
            res.body = strdup("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile Not Found");
            return res;
        }

        const char *content_type = get_content_type(filepath);
        size_t file_len = strlen(file_contents);

        // Allocate buffer for full response (headers + body)
        res.body = malloc(1024 + file_len);  // Plenty of space for headers
        if (res.body == NULL) {
            free(file_contents);
            res.body = strdup("HTTP/1.1 500 Internal Server Error\r\n\r\n");
            return res;
        }
        // Write headers into res.body
        int offset = snprintf(res.body, 1024,
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "\r\n",
                              content_type, file_len
        );

        // Append file contents
        strcpy(res.body + offset, file_contents);

        free(file_contents);

    } else if (strcmp(req->method, "POST") == 0) {
        handle_post_request(req, &res);
    } else {
        res.body = strdup("HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\n\r\nMethod Not Allowed");
        return res;
    }

    return res;
}

void free_response(Response *res) {
    if (res->body) free(res->body);
}