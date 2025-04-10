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

    //  Brute force split the request into headers and body:
    char const* idx = strstr(raw, "\r\n\r\n");

    if (idx) {
        long loc = idx - raw;
        strncpy(req.headers, raw, loc);

        idx += 4; // Move past the double CRLF
        strncpy(req.body, idx, sizeof(req.body) - 1);
        req.body[sizeof(req.body) - 1] = '\0'; // Null-terminate
    }

    //  TODO - Probably we could use a regex to split headers into key/value pairs!
    //    This would make our lives so much easier later.


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
