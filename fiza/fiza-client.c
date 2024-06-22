#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8083

int main() {
    int sock;
    struct sockaddr_in server_addr;
    int number, priority;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("192.168.0.1");  // Replace with your server's IP address
    
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    printf("Enter number and priority (negative priority to quit): ");
    scanf("%d %d", &number, &priority);
    
    if (number == 0) {
        printf("Number 0 is not allowed.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    send(sock, &number, sizeof(int), 0);
    send(sock, &priority, sizeof(int), 0);
    
    close(sock);
    return 0;
}