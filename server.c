#include "server.h"
#include "request.h"
#include "response.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
// No cleanup needed for Unix/macOS
#define CLEANUP_SOCKETS()
typedef int socket_t;

#endif

int start_server(int port) {

    // TODO - start the server
    //   remember the server flow:
    //   Use the provided macros to init/cleanup/close sockets
    //   1. create a socket
    //   2. bind the socket to an address
    //   3. listen for incoming connections
    //   4. accept incoming connections
    //   5. read the request
    //   6. parse the request
    //   7. handle the request (create response)
    //   8. send the response
    //   9. close the connection

    CLOSE_SOCKET(server_fd);
    CLEANUP_SOCKETS();
    return 0;
}











