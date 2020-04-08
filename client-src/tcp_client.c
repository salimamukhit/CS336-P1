#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#include "../shared-src/ini_parser.h"
#include "../shared-src/logger.h"
#include "../shared-src/structs.h"
#include "../shared-src/msleep.h"

#define PORT 50000   /* the port client will be connecting to */
#define MAXDATASIZE 100 /* max number of bytes we can get at once */
#define CONFIGNAME "config.ini"

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
    while(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        sleep(1);
        printf("Trying again in 1 seconds\n");
    }
    printf("Connection success\n");

    char line_arr[256] = { 0 };
    FILE* fp = fopen(CONFIGNAME, "r");
    while(1) {
        while(fgets(line_arr, 256, fp) != NULL) {
            LOG("LINE: %s", line_arr);
            if(send(sockfd, line_arr, 256, 0) == -1) {
                perror("send");
                exit(EXIT_FAILURE);
            }
            msleep(100);
        }
        printf("After the send function \n");
        if(send(sockfd, "EOF", 30, 0) == -1) {
            perror("EOF");
            exit(EXIT_FAILURE);
        }

        if((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
            perror("recv");
            exit(1);
        }	

        buf[numbytes] = '\0';

        printf("Received in pid=%d, text=: %s \n",getpid(), buf);
        break;
        sleep(1);
    }

    shtudown(sockfd);
    close(sockfd);
    return 0;
}

int receive_results(unsigned short int port, struct ini_info *info, double *low_arrival, double *high_arrival) {
    int sockfd;
    int numbytes;
    struct sockaddr_in servaddr;
    sleep(3);

     if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        return -1;
    }
    printf("Socket creation successful\n");

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr = info->server_ip;
    bzero(&(servaddr.sin_zero), 8);

    sleep(2);
    printf("Connecting to the server\n");
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1) {
        perror("Connection");
        return -1;
    } 
    printf("Connection was successful\n");

    char results[128];
    if((numbytes = recv(sockfd, results, sizeof(results), 0)) < 0) {
        perror("Couldn't receive the results");
        return -1;
    }
    results[numbytes] = '\0';
    printf("Received results: %s\n", results);

    close(sockfd);
    return 0;
}


