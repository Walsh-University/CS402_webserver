
#ifndef WEBSERVER_REQUEST_H
#define WEBSERVER_REQUEST_H

typedef struct {
    char method[8];
    char path[256];
    char version[16];
    char headers[512]; // Placeholder for headers
    char body[1024]; // Placeholder for body
} Request;

Request parse_request(const char *raw);

#endif //WEBSERVER_REQUEST_H
