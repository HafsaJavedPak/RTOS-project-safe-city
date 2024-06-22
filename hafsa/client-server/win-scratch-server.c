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
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    WSADATA wsa;
    SOCKET sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;
    int portno, n;
    int clilen;
    char buffer[255];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        error("Error opening socket\n");
    }

    memset((char *)&server_addr, 0, sizeof(server_addr));
    portno = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        error("Binding failed\n");
    }

    listen(sockfd, 5);
    clilen = sizeof(client_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);
    if (newsockfd == INVALID_SOCKET) {
        error("Error on accept\n");
    }

    while (1) {
        memset(buffer, 0, 255);
        n = recv(newsockfd, buffer, 255, 0);
        if (n == SOCKET_ERROR) {
            error("Error on read\n");
        }

        printf("Client: %s\n", buffer);

        // memset(buffer, 0, 255);
        // fgets(buffer, 255, stdin);

        // n = send(newsockfd, buffer, strlen(buffer), 0);
        // if (n == SOCKET_ERROR) {
        //     error("Error on write\n");
        // }

        int i = strncmp("Bye", buffer, 3);
        if (i == 0) {
            break;
        }
    }

    closesocket(newsockfd);
    closesocket(sockfd);
    WSACleanup();

    return 0;
}
