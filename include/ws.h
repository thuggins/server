#ifndef WS_H
#define WS_H

// Perform WebSocket handshake using HTTP request headers. Returns 1 on success, 0 on failure.
int websocket_handshake(int client, const char* request);

// Send a text message over WebSocket.
int websocket_send_text(int client, const char* msg);

// Read a text message from WebSocket.
int websocket_read_text(int client, char* out, int out_size);

#endif // WS_H
