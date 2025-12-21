
#ifndef WORKER_H
#define WORKER_H

#include <winsock2.h>
#include <ws2tcpip.h>

typedef struct {
    SOCKET client;
    struct sockaddr_in addr;
    void* ssl_ctx; // SSL_CTX* for SSL support, NULL for plain
} client_ctx_t;

#endif // WORKER_H

/**
@brief Thread proc that handles a single client connection and closes the socket.
@param arg Pointer to `client_ctx_t` allocated by the caller.
@return Thread exit code (0 when done).
*/
unsigned __stdcall client_worker(void* arg);
