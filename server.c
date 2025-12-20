#include <stdio.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create TCP socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    // Enable address reuse to avoid "address already in use" errors
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    // Configure server address (bind to all interfaces on port 8080)
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    // Bind socket to address
    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Bind failed\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Start listening for connections (queue up to 10)
    listen(sock, 10);
    printf("Server running on http://localhost:8080\n");
    printf("Press Ctrl+C to stop.\n");

    // HTML content to serve
    const char *html = "<!DOCTYPE html>"
                       "<html>"
                       "<head><title>Simple C Server</title></head>"
                       "<body>"
                       "<h1>Hello from C Web Server!</h1>"
                       "<p>This is a simple web server written in C.</p>"
                       "</body>"
                       "</html>";

    // Main server loop
    while (1)
    {
        // Accept incoming connection
        struct sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET client = accept(sock, (struct sockaddr *)&clientAddr, &clientLen);
        if (client == INVALID_SOCKET)
            continue;

        // Get client IP address for logging
        char clientIP[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN))
        {
            strcpy(clientIP, "unknown");
        }

        // Receive HTTP request
        char request[1024] = {0};
        int recvBytes = recv(client, request, sizeof(request) - 1, 0);
        if (recvBytes <= 0)
        {
            closesocket(client);
            continue;
        }

        // Extract first line of request for logging
        char requestLine[256] = {0};
        char *line_end = strstr(request, "\r\n");
        if (line_end)
        {
            int len = line_end - request;
            if (len > 255)
                len = 255;
            strncpy(requestLine, request, len);
        }

        // Build HTTP response
        char response[2048];
        int responseLen = sprintf(response,
                                  "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/html\r\n"
                                  "Content-Length: %d\r\n"
                                  "\r\n"
                                  "%s",
                                  (int)strlen(html), html);

        // Send response and log result
        int sent = send(client, response, responseLen, 0);
        if (sent > 0 && requestLine[0])
        {
            printf("[%s] %s - 200 OK\n", clientIP, requestLine);
        }
        else if (sent == SOCKET_ERROR && requestLine[0])
        {
            printf("[%s] %s - Send failed\n", clientIP, requestLine);
        }

        closesocket(client);
    }

    // Cleanup (unreachable in this infinite loop)
    closesocket(sock);
    WSACleanup();
    return 0;
}
