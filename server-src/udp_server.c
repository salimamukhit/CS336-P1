#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../shared-src/ini_parser.h"
#include "../shared-src/structs.h"
#include "../shared-src/logger.h"

int start_udp_server(struct ini_info *info) {
    char buf[8192] = { 0 };
    // Setup structures from our INI
    struct sockaddr_in recaddr; // Receiver's address, i.e. udp server
    struct sockaddr_in sendaddr; // Sender's address, i.e. udp client
    bzero(&recaddr, sizeof(recaddr));
    bzero(&sendaddr, sizeof(sendaddr));

    recaddr.sin_family = AF_INET; 
    recaddr.sin_addr = info->server_ip;
    recaddr.sin_port = info->train_udp.udph_destport;

    sendaddr.sin_family = AF_INET;
    sendaddr.sin_addr = info->server_ip;
    sendaddr.sin_port = info->train_udp.udph_srcport;

    printf("Sender port: %d Receiver port: %d\n", sendaddr.sin_port, recaddr.sin_port);

    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd < 0) {
        perror("Socket");
        return -1;
    }

    if(bind(sockfd, (struct sockaddr*) &recaddr, (socklen_t) sizeof(recaddr)) != 0) {
        perror("Bind");
        return -1;
    }

    LOGP("No errors so far!\n");
    
    recvfrom(sockfd, &buf, 8192, 0, &sendaddr, sizeof(sendaddr));
    LOG("Received %s\n", buf);
    
    sleep(3);
    /*for(int i=0; i<info->packet_num; i++) {
        recvfrom(sockfd, &buf, 8192, 0, &cliaddr.sin_addr, sizeof(cliaddr));
        LOG("Received %s\n", buf);
    }*/

    close(sockfd);

    return 0;
}