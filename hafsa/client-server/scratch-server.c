#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
// #include<netdb.h>



void error (const char *msg) {
    perror(msg);
    exit(1);
}

int main (int argc, char *argv[] ) {
    if (argc < 2) {
        error("port not provided\n");
    }

    int sockfd, newsockfd, portno, n;
    char buffer[255];

    struct sockaddr_in server_addr, client_addr;
    socklen_t clilen; //32 bit datatype, gives intrnet address 

    sockfd = socket(AF_INET, SOCK_STREAM,0);
    
    if (sockfd < 0) {
        error("Error openinf socket\n");
    }
    // bzero clear data or refee???
    bzero( (char *)&server_addr, sizeof(server_addr));
    portno = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // htno????
    server_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        error("Binding failed\n");
    }

    // now listinenign
    listen(sockfd, 5);
    clilen = sizeof(client_addr);


    newsockfd = accept(sockfd, (struct sockaddr*) &client_addr, &clilen);

    if (newsockfd < 0) {
        error("Errro on accept\n");
    }

    while ((1))
    {
        bzero(buffer, 255);
        // read from client
        n = read(newsockfd, buffer, 255);
        if (n < 0) {
            error("Errro on read\n");
        }

        printf("Client : %s\n",buffer);

        // bzero(buffer,255);
        // // stdio.h, read btes from string???
        // fgets(buffer, 250, stdin);
        
        // // write to client from server
        // // how does server write???
        // n = write(newsockfd, buffer, strlen(buffer));
        // if (n < 0) {
        //     error("Error on writng\n");
        // }

        int i = strncmp("Bye", buffer, 3);
        
        if (n==0) {
            break;
        }
    }   
    
    close(newsockfd);
    close(sockfd);

    return 0;
}
