#include "request.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#define MAX_METHOD_LEN 8
#define MAX_PATH_LEN 256
#define MAX_VERSION_LEN 16




Request parse_request(const char *raw) {
    Request req = {0};
    sscanf(raw, "%7s %255s %15s", req.method, req.path, req.version);

    const char *header_start = strstr(raw, "\r\n");
    const char *body_start = strstr(raw, "\r\n\r\n");

    if (!header_start || !body_start) {
        return req; // Malformed request
    }

    header_start += 2; // Move past request line CRLF

    // Copy headers into a temporary buffer
    size_t headers_len = body_start - header_start;
    char headers_buf[1024];
    strncpy(headers_buf, header_start, headers_len);
    headers_buf[headers_len] = '\0';

    // Split headers by line using strtok_r
    char *saveptr_line;
    char const *line = strtok_r(headers_buf, "\r\n", &saveptr_line);

    while (line && req.header_count < MAX_HEADERS) {
        char const *colon = strchr(line, ':');
        if (colon) {
            size_t key_len = colon - line;
            size_t value_len = strlen(colon + 1);

            // Trim key
            strncpy(req.header_list[req.header_count].key, line, key_len);
            req.header_list[req.header_count].key[key_len] = '\0';

            // Trim value
            const char *value_start = colon + 1;
            while (*value_start == ' ') value_start++;  // Skip leading space

            strncpy(req.header_list[req.header_count].value, value_start, MAX_HEADER_VALUE - 1);
            req.header_list[req.header_count].value[MAX_HEADER_VALUE - 1] = '\0';

            req.header_count++;
        }
        line = strtok_r(nullptr, "\r\n", &saveptr_line);
    }

    // Copy body
    const char *body = body_start + 4;
    strncpy(req.body, body, sizeof(req.body) - 1);
    req.body[sizeof(req.body) - 1] = '\0';

    return req;
}


Request parse_request_safe(const char *raw) {
    Request req = {0};
    char buffer[1024];

    // Copy raw input to buffer
    strncpy(buffer, raw, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Use sscanf to parse the buffer safely
    sscanf(buffer, "%7s %255s %15s", req.method, req.path, req.version);

    // Basic sanity check
    if (strlen(req.method) == 0 || strlen(req.path) == 0) {
        strcpy(req.method, "INVALID");
    }

    return req;
}

Request parse_request_safer(const char *raw) {
    Request req = {0};
    char buffer[1024];

    // Copy raw input to buffer
    strncpy(buffer, raw, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Use sscanf to parse the buffer safely
    if (sscanf(buffer, "%7s %255s %15s", req.method, req.path, req.version) != 3) {
        strcpy(req.method, "INVALID");
        return req;
    }

    // Basic sanity check
    if (strlen(req.method) == 0 || strlen(req.path) == 0) {
        strcpy(req.method, "INVALID");
    }

    // Additional validation
    for (size_t i = 0; i < strlen(req.method); i++) {
        if (!isalpha(req.method[i])) {
            strcpy(req.method, "INVALID");
            break;
        }
    }

    return req;
}
