#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    int portno;
    char buffer[255];
    int n;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        exit(1);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        error("Error opening socket\n");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        WSACleanup();
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        error("Connection failed\n");
    }

    while (1) {
        memset(buffer, 0, 255);
        fgets(buffer, 255, stdin);

        n = send(sockfd, buffer, strlen(buffer), 0);
        if (n == SOCKET_ERROR) {
            error("Error on writing\n");
        }

        // memset(buffer, 0, 255);
        // n = recv(sockfd, buffer, 255, 0);
        // if (n == SOCKET_ERROR) {
        //     error("Error on reading\n");
        // }

        printf("Server: %s\n", buffer);

        if (strncmp("Bye", buffer, 3) == 0) {
            break;
        }
    }

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
