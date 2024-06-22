#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>

// Function to handle client communication
void* handle_client(void* client_socket) {
    int client_sock = *((int*)client_socket);
    free(client_socket);
    char buffer[1024];
    int n;

    printf("[+]Client connected.\n");

    bzero(buffer, 1024);
    recv(client_sock, buffer, sizeof(buffer), 0);
    printf("Client: %s\n", buffer);

    bzero(buffer, 1024);
    strcpy(buffer, "HI, THIS IS SERVER. HAVE A NICE DAY!!!");
    printf("Server: %s\n", buffer);
    send(client_sock, buffer, strlen(buffer), 0);

    close(client_sock);
    printf("[+]Client disconnected.\n");

    return NULL;
}

// Function to list all available IP addresses
void list_ip_addresses() {
    struct ifaddrs *addrs, *tmp;
    char ip[INET_ADDRSTRLEN];

    if (getifaddrs(&addrs) == -1) {
        perror("getifaddrs");
        exit(1);
    }

    tmp = addrs;

    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            inet_ntop(AF_INET, &pAddr->sin_addr, ip, sizeof(ip));
            printf("%s: %s\n", tmp->ifa_name, ip);
        }

        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
}

int main() {
    // List available IP addresses
    list_ip_addresses();

    int port = 5566;

    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    pthread_t tid;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to all available interfaces

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to the port number: %d\n", port);

    listen(server_sock, 5);
    printf("Listening...\n");

    while (1) {
        addr_size = sizeof(client_addr);
        int* client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (*client_sock < 0) {
            perror("[-]Accept error");
            free(client_sock);
            continue;
        }

        pthread_create(&tid, NULL, handle_client, client_sock);
        pthread_detach(tid);  // Detach the thread to reclaim resources when it finishes
    }

    close(server_sock);
    return 0;
}
