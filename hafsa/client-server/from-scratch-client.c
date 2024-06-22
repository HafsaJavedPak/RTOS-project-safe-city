#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>



void error (const char *msg) {
    perror(msg);
    exit(1);
}

int main (int argc, char *argv ) {
    if (argc < 3) {
        error("ip adress or port num not provided\n");
    }

    int sockfd, newsockfd, portno, n;
    char buffer[255];

    struct sockaddr_in server_addr;
    struct hostent *server;
    
    
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM,0);

    if (sockfd < 0) {
        error("Error openinf socket\n");
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        // ???frptintf
        error("Error, no such host");
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // it will copy n bytes from server to  serv_adress 
    // ???? h_addrr what???
    bcopy((char *) server->h_addr, (char *) & server_addr.sin_addr.s_addr, server->h_length);

    server_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)  &server_addr, sizeof(server_addr) < 0)) {
        error("Connection failed\n");
    }

    while(1) {
        bzero(buffer, 255);
        fgets(buffer, 255, stdin);

        n = write(sockfd, buffer, strlen(buffer));

        if (n < 0) {
            error("Error on wroting\n");
        }

        bzero(buffer, 255);

        n = read(sockfd, buffer, 255);

        if (n < 0) {
            error("Error on reading\n");
        }

        printf("Server : %s\n", buffer);

        int i = strncmp("Bye", buffer, 3);
        
        if (n==0) {
            break;
        }
    }   
    
    close(sockfd);

    return 0;
}