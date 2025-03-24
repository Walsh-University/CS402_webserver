
#ifndef WEBSERVER_REQUEST_H
#define WEBSERVER_REQUEST_H

typedef struct {
    char method[8];
    char path[256];
    char version[16];
} Request;

Request parse_request(const char *raw);

#endif //WEBSERVER_REQUEST_H
