#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "udp_server.h"
#include "../shared-src/ini_parser.h"
#include "../shared-src/structs.h"
#include "../shared-src/logger.h"

#define PORT 8080

int start_udp_server(struct ini_info *info) {
    printf("We got to starting the UDP server!\n");
    printf("The port is: %d\n", htons(PORT));

    char buffer[info->payload_size]; // storage of received data
    struct sockaddr_in servaddr, cliaddr; // declaring server and client addresses

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // creating socket
    if(sockfd < 0) {
        perror("Socket");
        return -1;
    } 
    printf("Socket creation was successful\n");
    
    memset(&servaddr, 0, sizeof(servaddr)); // zeroing cliaddr and servaddr memory
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET; // filling servaddr with config data
    servaddr.sin_addr.s_addr = info->server_ip.s_addr;
    servaddr.sin_port = htons(PORT);

    printf("Address is %d\n", info->server_ip.s_addr);

    if(bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) { // binding
        perror("Bind");
        return -1;
    }
    printf("Binding was successful\n");

    unsigned int cliaddr_len = sizeof(cliaddr);
    int n;

    n = recvfrom(sockfd, &buffer, sizeof(buffer), MSG_WAITALL, 
    (struct sockaddr *)&cliaddr, &cliaddr_len);
    buffer[n] = '\0';

    printf("Received data: %s\n", buffer);

    //int packets = info->packet_num;
    int packets = 2;

    for(int i = 0; i < packets*2; i++) {
        n = recvfrom(sockfd, &buffer, sizeof(buffer), MSG_WAITALL, 
        (struct sockaddr *)&cliaddr, &cliaddr_len);
        buffer[n] = '\0';
        printf("Received data: %s\n", buffer);
    }

    close(sockfd);
    
    return 0;
}