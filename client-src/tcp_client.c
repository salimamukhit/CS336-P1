#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#include "ini_parser.h"

#define PORT 50000   /* the port client will be connecting to */
#define MAXDATASIZE 100 /* max number of bytes we can get at once */

// Declare struct prototype for struct ini_info
//struct ini_info;

int send_config(struct ini_info *info) {
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;      /* host byte order */
    their_addr.sin_port = htons(info->server_port);    /* short, network byte order */
    their_addr.sin_addr = info->server_ip;
    bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

    printf("Preparing the connection...\n");
    if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    } else {
        printf("Connection success");
    }
    while(1) {
        if(send(sockfd, "Hello, world!\n", 14, 0) == -1){
            perror("send");
            exit (1);
        }
        printf("After the send function \n");

        if((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
            perror("recv");
            exit(1);
        }	

            buf[numbytes] = '\0';

            printf("Received in pid=%d, text=: %s \n",getpid(), buf);
        sleep(1);
    }

    close(sockfd);

    return 0;
}


