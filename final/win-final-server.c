#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <pthread.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 10

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    SOCKET sockfd;
    int priority;
} client_info;

client_info clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t lock;

void *handle_client(void *arg) {
    int n;
    char buffer[255];
    client_info *client = (client_info *)arg;

    while (1) {
        memset(buffer, 0, 255);
        n = recv(client->sockfd, buffer, 255, 0);
        if (n == SOCKET_ERROR) {
            error("Error reading from socket");
        }
        if (n == 0) {
            break;
        }

        int priority = atoi(buffer);
        if (priority < 0 || priority > 3) {
            printf("Invalid priority received\n");
            continue;
        }

        pthread_mutex_lock(&lock);
        client->priority = priority;
        pthread_mutex_unlock(&lock);

        printf("Received from client (priority %d): %s\n", client->priority, buffer);
    }

    closesocket(client->sockfd);
    return NULL;
}

void schedule_clients() {
    int round_robin_index = 0;

    while (1) {
        pthread_mutex_lock(&lock);
        int highest_priority = -1;
        int highest_priority_index = -1;
        int same_priority_count = 0;

        for (int i = 0; i < client_count; i++) {
            if (clients[i].priority > highest_priority) {
                highest_priority = clients[i].priority;
                highest_priority_index = i;
                same_priority_count = 1;
            } else if (clients[i].priority == highest_priority) {
                same_priority_count++;
            }
        }

        if (highest_priority != -1) {
            if (same_priority_count > 1) {
                // Round-robin scheduling for clients with the same priority
                int start_index = round_robin_index % same_priority_count;
                int handled_client_index = -1;
                for (int i = 0; i < client_count; i++) {
                    if (clients[i].priority == highest_priority) {
                        if (start_index == 0) {
                            handled_client_index = i;
                            break;
                        }
                        start_index--;
                    }
                }
                printf("Handling priority %d client\n", clients[handled_client_index].priority);
                clients[handled_client_index].priority = -1; // Reset priority
                round_robin_index++;
            } else {
                printf("Handling priority %d client\n", clients[highest_priority_index].priority);
                clients[highest_priority_index].priority = -1; // Reset priority
            }
        }
        pthread_mutex_unlock(&lock);
        Sleep(10); // Sleep to prevent busy waiting
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    WSADATA wsa;
    SOCKET sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;
    int portno;
    int clilen;
    pthread_t threads[MAX_CLIENTS];

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        error("Failed to initialize Winsock");
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        error("Error opening socket");
    }

    memset((char *)&server_addr, 0, sizeof(server_addr));
    portno = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        error("Error on binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(client_addr);

    pthread_mutex_init(&lock, NULL);

    // Start the scheduler thread
    pthread_t scheduler_thread;
    pthread_create(&scheduler_thread, NULL, (void *(*)(void *))schedule_clients, NULL);

    while (client_count < MAX_CLIENTS) {
        newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);
        if (newsockfd == INVALID_SOCKET) {
            error("Error on accept");
        }
        clients[client_count].sockfd = newsockfd;
        clients[client_count].priority = -1;
        pthread_create(&threads[client_count], NULL, handle_client, (void *)&clients[client_count]);
        client_count++;
    }

    for (int i = 0; i < client_count; i++) {
        pthread_join(threads[i], NULL);
    }

    closesocket(sockfd);
    pthread_mutex_destroy(&lock);
    WSACleanup();

    return 0;
}
