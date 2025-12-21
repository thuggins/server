#ifndef WS_H
#define WS_H

#include <openssl/ssl.h>
#include <winsock2.h>

// Perform WebSocket handshake using HTTP request headers. Returns 1 on success, 0 on failure.
int websocket_handshake_ssl(SSL* ssl, const char* request);

// Send a text message over WebSocket (SSL).
int websocket_send_text_ssl(SSL* ssl, const char* msg);

// Read a text message from WebSocket (SSL).
int websocket_read_text_ssl(SSL* ssl, char* out, int out_size);

#endif // WS_H
