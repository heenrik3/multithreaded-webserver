#include <winsock2.h>
#include <stdio.h>

#include "pthread.h"

#define PORT 8080

void *routine(void* socketFD)
{
    SOCKET socket = *(SOCKET*)socketFD;
    free(socketFD);

    char request[4096];
    char response[4096];

    // Receive data from the client
    int receiveResult = recv(socket, request, sizeof(request), 0);
    if (receiveResult == SOCKET_ERROR) {
        printf("Failed to receive data from client.\n");
        closesocket(socket);
    }

    printf("[REQUEST] %s\n", request);

    // Send a response to the client
    char markup[] = "<!DOCTYPE html>"
                    "<html lang=\"pt-BR\">"
                        "<head>"
                            "<meta charset=\"UTF-8\" />"
                            "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />"
                            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />"
                            "<title>Página Teste</title>"
                        "</head>"
                        "<body style=\"margin: 0; padding: 0; height: 100vh; display: flex; align-items: center; justify-content: center; font-family: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;\">"
                            "<h1 style=\"font-size: 64px; text-align:center;\">Olá do servidor!</h1>"
                        "</body>"
                    "</html>";
    
    
    snprintf(response, 
    sizeof(response), 
    "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n%s",
     strlen(markup), markup);

    int sendResult = send(socket, response, strlen(response), 0);
    if (sendResult == SOCKET_ERROR) {
        printf("Failed to send response to client.\n");
    }

    closesocket(socket);    

    return NULL;
}

void handleNewConnection(SOCKET* socketFD)
{

    printf("[CONNECTION]\n");

    pthread_t thread;

    pthread_create(&thread, NULL, routine, socketFD);
    
    pthread_detach(thread);
}

int main() {
    WSADATA wsaData;
    SOCKET listenSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock.\n");
        return 1;
    }

    // Create a socket to listen for incoming connections
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("Failed to create listening socket.\n");
        return 1;
    }

    // Bind the socket to a local address and port
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);
    if (bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Failed to bind socket to local address and port.\n");
        return 1;
    }

    // Start listening for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Failed to start listening for incoming connections.\n");
        return 1;
    }

    printf("Server running on port %d...\n", PORT);

    // Loop to handle incoming connections
    while (1) {

        SOCKET *clientSocket = malloc(sizeof(SOCKET));
        if (!clientSocket) {
            printf("Failed to allocate socket.\n");
            continue;
        }

        // Accept an incoming connection
        *clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
        if (*clientSocket == INVALID_SOCKET) {
            printf("Failed to accept incoming connection.\n");
            continue;
        }

        handleNewConnection(clientSocket);
    }

    // Clean up the listening socket
    closesocket(listenSocket);

    // Clean up Winsock
    WSACleanup();

    return 0;
}
