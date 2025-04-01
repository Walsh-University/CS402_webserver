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

Response handle_request(const Request *req) {
    Response res = {0};

    if (strcmp(req->method, "GET") != 0) {
        res.body = strdup("HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return res;
    }

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
    return res;
}

void free_response(Response *res) {
    if (res->body) free(res->body);
}