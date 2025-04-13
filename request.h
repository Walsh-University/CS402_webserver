
#ifndef WEBSERVER_REQUEST_H
#define WEBSERVER_REQUEST_H

#define MAX_HEADERS 32
#define MAX_HEADER_KEY 64
#define MAX_HEADER_VALUE 256

typedef struct {
    char key[MAX_HEADER_KEY];
    char value[MAX_HEADER_VALUE];
} Header;

typedef struct {
    char method[8];
    char path[256];
    char version[16];
    char body[1024];         // Placeholder for body
    int  header_count;
    Header header_list[MAX_HEADERS];
} Request;

Request parse_request(const char *raw);

#endif //WEBSERVER_REQUEST_H
