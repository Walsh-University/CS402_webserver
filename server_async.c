/*
 * This file is a simple HTTP server implementation with non-blocking sockets
 * */
#include "request.h"
#include "response.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define INIT_SOCKETS() { WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData); }
    #define CLOSE_SOCKET(s) closesocket(s)
    #define CLEANUP_SOCKETS() WSACleanup()
    typedef SOCKET socket_t;
    #define SET_NONBLOCKING(s) { u_long mode = 1; ioctlsocket(s, FIONBIO, &mode); }
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#define INIT_SOCKETS()
#define CLOSE_SOCKET(s) close(s)
#define CLEANUP_SOCKETS()
typedef int socket_t;
#define SET_NONBLOCKING(s) { int flags = fcntl(s, F_GETFL, 0); fcntl(s, F_SETFL, flags | O_NONBLOCK); }
#endif

#define BACKLOG 10

int start_server_async(int port) {
    INIT_SOCKETS();

    socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1 ||
        listen(server_fd, BACKLOG) == -1) {
        perror("bind or listen");
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    //
    // Set the server socket to non-blocking mode
    //
    SET_NONBLOCKING(server_fd);
    log_info("Select-based simple server started on port %d", port);

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        // Set a timeout for select
        struct timeval timeout = { .tv_sec = 1, .tv_usec = 0 }; // 1-second timeout
        // Wait for incoming connections
        int ready = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(server_fd, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            socket_t client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd == -1) {
                perror("accept");
                continue;
            }

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            log_info("Connection accepted from %s", client_ip);

            char buffer[1024] = {0};
            size_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

            if (bytes_read == -1) {
                perror("Error reading request");
                CLOSE_SOCKET(client_fd);
                continue;
            }

            Request req = parse_request(buffer);
            Response res = handle_request(&req);
            write(client_fd, res.body, strlen(res.body));
            CLOSE_SOCKET(client_fd);
            free_response(&res);
        }
    }

    CLOSE_SOCKET(server_fd);
    CLEANUP_SOCKETS();
    return 0;
}
