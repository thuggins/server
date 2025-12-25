
#ifndef WORKER_H
#define WORKER_H

#include <netinet/in.h>

typedef struct {
    int client;
    struct sockaddr_in addr;
} client_ctx_t;

#endif // WORKER_H

/**
@brief Thread proc that handles a single client connection and closes the socket.
@param arg Pointer to `client_ctx_t` allocated by the caller.
@return Thread exit code (0 when done).
*/
void* client_worker(void* arg);
