#include "server.h"
#include "server_async.h"
#include "server_threaded.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080

void start_web_server() {
    start_server(PORT);
}

void start_web_server_async() {
    start_server_async(PORT);
}

void start_web_server_threaded() {
    start_server_threaded(PORT);
}


int main(int argc, char *argv[]) {
    if (argc == 1) {
        start_web_server();
    } else if (argc == 2) {
        if (strcmp(argv[1], "a") == 0) {
            start_web_server_async();
        }
        else if (strcmp(argv[1], "t") == 0) {
            start_web_server_threaded();
        } else {
            fprintf(stderr, "Usage: %s [a|t]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}








