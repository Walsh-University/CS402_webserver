/*
 * This file is a simple HTTP server implementation with a thread-per-connection
 * */
#include "server_threaded.h"
#include "request.h"
#include "response.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
        #include <ws2tcpip.h>
        // Windows needs to link with Winsock Library
        #pragma comment(lib, "ws2_32.lib")

        #define INIT_SOCKETS() \
                { \
                    WSADATA wsaData; \
                    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { \
                        fprintf(stderr, "WSAStartup failed\n"); \
                        exit(1); \
                    } \
                }
        #define CLOSE_SOCKET(s) closesocket(s)
        #define CLEANUP_SOCKETS() WSACleanup()
        typedef SOCKET socket_t;

#else
    // Unix/macOS headers
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>

    #define INIT_SOCKETS() /* nothing to do */
    #define CLOSE_SOCKET(s) close(s)
    #define SOCKET_ERRNO errno
    #define CLEANUP_SOCKETS()
    typedef int socket_t;

#endif



#define BACKLOG 10

// Structure to pass client data to thread
typedef struct {
    socket_t client_fd;
    char client_ip[INET_ADDRSTRLEN];
} client_info;

// Thread function to handle client request
void* handle_client(void* arg) {
    client_info* info = (client_info*)arg;
    socket_t client_fd = info->client_fd;

    log_info("Thread handling connection from %s", info->client_ip);

    // Read request from client
    char buffer[1024] = {0};
    size_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("Error reading request");
        CLOSE_SOCKET(client_fd);
        free(info);
        pthread_exit(NULL);
    }

    // Parse request and handle it
    Request req = parse_request(buffer);
    Response res = handle_request(&req);

    // Send response to client
    write(client_fd, res.body, strlen(res.body));

    // Clean up
    free_response(&res);
    CLOSE_SOCKET(client_fd);
    free(info);

    pthread_exit(NULL);
}

int start_server_threaded(int port) {
    INIT_SOCKETS();

    socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // Set socket option to allow address reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen");
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    log_info("Threaded server (POSIX) started on port %d", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        socket_t client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        // Allocate client info struct
        client_info* info = malloc(sizeof(client_info));
        if (!info) {
            perror("malloc");
            CLOSE_SOCKET(client_fd);
            continue;
        }

        info->client_fd = client_fd;

        // Get client IP address
        inet_ntop(AF_INET, &client_addr.sin_addr, info->client_ip, sizeof(info->client_ip));
        log_info("Connection accepted from %s", info->client_ip);

        // Create a new thread to handle the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, info) != 0) {
            perror("pthread_create");
            free(info);
            CLOSE_SOCKET(client_fd);
            continue;
        }

        // Detach thread to free resources automatically when it exits
        //   handle_client will free the info struct and close the socket
        pthread_detach(thread_id);
    }

    CLOSE_SOCKET(server_fd);
    CLEANUP_SOCKETS();
    return 0;
}
