/**
@file http.h
@brief Public API for HTTP static serving helpers.

Serves responses from the `public/` directory.
*/
#pragma once
#include <winsock2.h>

/**
@brief Send a 200 OK response with the provided body.
@param client Connected client socket.
@param content_type MIME type for the body.
@param data Pointer to body bytes.
@param len Length of body in bytes.
*/
void http_send_response(SOCKET client, const char* content_type, const unsigned char* data,
                        size_t len);

/**
@brief Send a simple 404 Not Found HTML response.
@param client Connected client socket.
*/
void http_send_404(SOCKET client);

/**
@brief Serve a file by HTTP path from `public/`.
@param client Connected client socket.
@param path HTTP path (e.g., "/", "/app.js").
*/
void http_serve_file(SOCKET client, const char* path);
