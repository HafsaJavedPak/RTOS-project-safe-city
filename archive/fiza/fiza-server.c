#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <winsock2.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8083
#define MAX_CLIENTS 10

typedef struct {
    int number;
    int priority;
} Task;

typedef struct {
    Task tasks[MAX_CLIENTS];
    int size;
} PriorityQueue;

PriorityQueue pq;
pthread_mutex_t lock;
pthread_cond_t cond;
int task_count = 0;

void swap(Task *a, Task *b) {
    Task temp = *a;
    *a = *b;
    *b = temp;
}

void push(PriorityQueue *pq, int number, int priority) {
    if (pq->size == MAX_CLIENTS) {
        printf("Priority Queue is full!\n");
        return;
    }
    Task task = {number, priority};
    pq->tasks[pq->size++] = task;
    
    int i = pq->size - 1;
    while (i > 0 && pq->tasks[i].priority > pq->tasks[(i - 1) / 2].priority) {
        swap(&pq->tasks[i], &pq->tasks[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

Task pop(PriorityQueue *pq) {
    if (pq->size == 0) {
        printf("Priority Queue is empty!\n");
        exit(EXIT_FAILURE);
    }
    Task topTask = pq->tasks[0];
    pq->tasks[0] = pq->tasks[--pq->size];
    
    int i = 0;
    while (2 * i + 1 < pq->size) {
        int j = 2 * i + 1;
        if (j + 1 < pq->size && pq->tasks[j].priority < pq->tasks[j + 1].priority) {
            j++;
        }
        if (pq->tasks[i].priority >= pq->tasks[j].priority) break;
        swap(&pq->tasks[i], &pq->tasks[j]);
        i = j;
    }
    
    return topTask;
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    
    while (1) {
        int number, priority;
        int read_size = recv(client_socket, &number, sizeof(int), 0);
        if (read_size <= 0) {
            break;
        }
        read_size = recv(client_socket, &priority, sizeof(int), 0);
        if (read_size <= 0) {
            break;
        }

        printf("Received number: %d with priority: %d\n", number, priority);
        
        pthread_mutex_lock(&lock);
        if (priority == 0) {
            printf("No gesture\n");
        } else {
            push(&pq, number, priority);
            task_count++;
            if (task_count >= 5) {
                pthread_cond_signal(&cond);
            }
        }
        pthread_mutex_unlock(&lock);
    }
    
    close(client_socket);
    return NULL;
}

void *process_tasks(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (task_count < 5) {
            pthread_cond_wait(&cond, &lock);
        }
        Task task = pop(&pq);
        task_count = 0;
        pthread_mutex_unlock(&lock);
        printf("Processed number: %d with priority: %d\n", task.number, task.priority);
    }
    return NULL;
}

int main() {
    int server_socket, client_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    pq.size = 0;
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all network interfaces
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    listen(server_socket, 3);
    
    printf("Server listening on port %d\n", PORT);
    
    pthread_t process_thread;
    if (pthread_create(&process_thread, NULL, process_tasks, NULL) < 0) {
        perror("Could not create processing thread");
        exit(EXIT_FAILURE);
    }
    
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len))) {
        printf("Connection accepted\n");
        
        pthread_t client_thread;
        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;
        
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_sock) < 0) {
            perror("Could not create thread");
            free(new_sock);
        }
    }
    
    if (client_socket < 0) {
        perror("Accept failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    return 0;
}