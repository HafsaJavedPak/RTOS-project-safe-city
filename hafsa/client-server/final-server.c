#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    int sockfd;
    int priority;
} client_info;

client_info clients[2];
pthread_mutex_t lock;

void *handle_client(void *arg) {
    int n;
    char buffer[255];
    client_info *client = (client_info *)arg;

    while (1) {
        bzero(buffer, 255);
        n = read(client->sockfd, buffer, 255);
        if (n < 0) {
            error("Error reading from socket");
        }
        if (n == 0) {
            break;
        }

        // Extract priority
        int priority = atoi(buffer);
        if (priority < 0 || priority > 3) {
            printf("Invalid priority received\n");
            continue;
        }

        pthread_mutex_lock(&lock);
        client->priority = priority;
        pthread_mutex_unlock(&lock);

        // Process message
        // for (int i = 0 ; i < 100000; i++) {
        //     if (i %200 == 0) {
        //         printf("i : %d\n",i);
        //     }
        // }
        printf("Received from client (priority %d): %s\n", client->priority, buffer);
    }

    close(client->sockfd);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int sockfd, portno;
    struct sockaddr_in server_addr, client_addr;
    socklen_t clilen;
    pthread_t threads[2];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    portno = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Error on binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(client_addr);

    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < 2; i++) {
        clients[i].sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);
        if (clients[i].sockfd < 0) {
            error("Error on accept");
        }
        clients[i].priority = -1;
        pthread_create(&threads[i], NULL, handle_client, (void *)&clients[i]);
    }

    while (1) {
        pthread_mutex_lock(&lock);
        if (clients[0].priority > clients[1].priority) {
            // Handle client 0's message
            printf("Handling priority %d client\n", clients[0].priority);

            clients[0].priority = -1; // Reset priority
        } else if (clients[1].priority > clients[0].priority) {
            // Handle client 1's message
            printf("Handling priority %d client\n", clients[1].priority);

            clients[1].priority = -1; // Reset priority
        }
        pthread_mutex_unlock(&lock);
    }

    close(sockfd);
    pthread_mutex_destroy(&lock);

    return 0;
}
