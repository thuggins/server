#include <openssl/ssl.h>
int websocket_handshake_ssl(SSL* ssl, const char* request);
int websocket_send_text_ssl(SSL* ssl, const char* msg);
int websocket_read_text_ssl(SSL* ssl, char* out, int out_size);
/**
@file ws.h
@brief Public API for WebSocket utilities (RFC 6455).

Notes:
- Server frames are unmasked; client frames must be masked.
*/
#pragma once
#include <winsock2.h>

/**
@brief Perform WebSocket handshake using HTTP request headers.
@param client Connected client socket.
@param request Raw HTTP upgrade request.
@return 1 on success (response written to socket), 0 on failure.
*/
int websocket_handshake(SOCKET client, const char* request);

/**
@brief Send a text frame to the client.
@param client Connected client socket.
@param msg Null-terminated UTF-8 text.
@return Bytes sent or -1 on error.
*/
int websocket_send_text(SOCKET client, const char* msg);

/**
@brief Read a client text frame into `out`.
@param client Connected client socket.
@param out Output buffer for decoded text (NUL-terminated).
@param out_size Size of `out` buffer.
@return >0 length of text payload; 0 on close; -1 on error/invalid; -2 if control frame handled
(e.g., ping).
*/
int websocket_read_text(SOCKET client, char* out, int out_size);
