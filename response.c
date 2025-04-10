#include "response.h"
#include "file_handler.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void handle_form_post(const Request *req, Response *res) {
    // TODO - Implement form handling logic
}

void handle_json_post(const Request *req, Response *res) {
    // TODO - Implement JSON handling logic

}

void handle_post_request(const Request *req, Response *res) {

    // First we need to know what type of data we're handling:
    // Content-Type: application/x-www-form-urlencoded/r/nblahblahblahablah

    char const* idx = strstr(req->headers, "Content-Type: ");
    char const* header = strstr(idx, "\r\n");
    long loc = header - idx;

    char content_type[100];
    strncpy(content_type, idx, loc);

    idx = strstr(content_type, ": ");
    idx += 2; // Move past ": "


    if (strstr(idx, "form") != nullptr ){
        handle_form_post(req, res);
    }
    else if (strstr(idx, "json") != nullptr) {
        handle_json_post(req, res);
    } else {
        // Handle other content types or return an error
        res->body = strdup("HTTP/1.1 415 Unsupported Media Type\r\n\r\n");
        return;
    }


    res = malloc(sizeof(Response));
    res->body = nullptr;
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
        res.body = strdup("HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return res;
    }

    return res;
}

void free_response(Response *res) {
    if (res->body) free(res->body);
}