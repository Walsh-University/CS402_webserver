#ifndef WEBSERVER_RESPONSE_H
#define WEBSERVER_RESPONSE_H
#include "request.h"

typedef struct {
    char *body;
} Response;

Response handle_request(const Request *req);
void free_response(Response *res);

#endif //WEBSERVER_RESPONSE_H
