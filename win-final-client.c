#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

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
    char buffer[255];
    int portno, n;

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        error("Failed to initialize Winsock");
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        error("Error opening socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        exit(0);
    }

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    server_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Error connecting");
    }

    while (1) {
        printf("Enter priority (0-3): ");
        memset(buffer, 0, 255);
        fgets(buffer, 255, stdin);

        n = send(sockfd, buffer, strlen(buffer), 0);
        if (n == SOCKET_ERROR) {
            error("Error writing to socket");
        }

        int i = strncmp("Bye", buffer, 3);
        if (i == 0) {
            break;
        }
    }

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
