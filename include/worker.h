/**
@file worker.h
@brief Public API for per-connection worker thread and context.

Provides:
- Context type for client sockets
- Thread proc to handle HTTP/WebSocket per connection
*/
#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>

/**
@brief Per-connection context passed to the worker thread.
*/
typedef struct {
    SOCKET client;
    struct sockaddr_in addr;
} client_ctx_t;

/**
@brief Thread proc that handles a single client connection and closes the socket.
@param arg Pointer to `client_ctx_t` allocated by the caller.
@return Thread exit code (0 when done).
*/
unsigned __stdcall client_worker(void* arg);
