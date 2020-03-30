#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../shared-src/ini_parser.h"
#include "../shared-src/logger.h"

int start_udp_server(struct ini_info *info) {
    char buf[8192] = { 0 };
    // Setup structures from our INI
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr = info->server_ip;
    servaddr.sin_port = htons(info->server_port);

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr = info->server_ip;
    cliaddr.sin_port = htons(info->server_port);

    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd < 0) {
        perror("Socket");
        return -1;
    }

    if(bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) != 0) {
        perror("Bind");
        return -1;
    }

    LOGP("No errors so far!\n");

    if(listen(sockfd, info->packet_num) != 0) {
        perror("Listen");
        return -1;
    } else {
        printf("Listening was successful!\n");
    }
    return 0;
}