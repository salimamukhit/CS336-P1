#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "udp_server.h"
#include "../shared-src/ini_parser.h"
#include "../shared-src/structs.h"
#include "../shared-src/logger.h"

#define PORT 8080
#define TIMEOUT 20

int start_udp_server(struct ini_info *info, double* low_arrival, double* high_arrival) {
    printf("UDP server started\n");
    printf("The port is: %d\n", htons(PORT));\
    printf("The number of packets is %d", info->packet_num);

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

    if(bind(sockfd, (const struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) { // binding
        perror("Bind");
        return -1;
    }
    printf("Binding was successful\n");

    // Setting up the time out for a socket (in seconds) We base this on the inter-measurement time
    // Defaulting it to two seconds, if the measurement time is not greater than two then set it to one.
    struct timeval tv;
    if(info->meas_time > 2) tv.tv_sec = 2;
    else tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    unsigned int cliaddr_len = sizeof(cliaddr);
    int n;

    n = recvfrom(sockfd, &buffer, sizeof(buffer), MSG_WAITALL, 
    (struct sockaddr *)&cliaddr, &cliaddr_len);
    buffer[n] = '\0';

    printf("Received data: %s\n", buffer);

    int packets = info->packet_num;

    /* Start the counting of the packets and the timer for the low entropy data */

    int count = 0;
    clock_t low_start, low_end, high_start, high_end;
    double low_time, high_time;

    n = recvfrom(sockfd, &buffer, sizeof(buffer), MSG_WAITALL, 
        (struct sockaddr *)&cliaddr, &cliaddr_len);
    buffer[n] = '\0';
    count++;
    low_start = clock();

    // for(int i = 0; i < packets-1; i++) {
    
    for(int i = 0; i < packets-1; i++) {
        n = recvfrom(sockfd, &buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *)&cliaddr, &cliaddr_len);
        if(n >= 0) {
            buffer[n] = '\0';
            count++;
            low_end = clock();
        }
        else if(i == 0 && n == -1) {
            fprintf(stderr, "Failed to receive any Low Entropy packets!\n");
            free(info);
            exit(EXIT_FAILURE);
        }
        else break;
    }
    printf("Received: %d\n", count);

    low_time = (low_end - low_start) / (double) CLOCKS_PER_SEC;
    printf("Low entropy arrival time: %lf sec\n", low_time);
    printf("first packet, last packet: %ld %ld\n", low_start, low_end);

    /* Start the counting of the packets and the timer for the high entropy data */

    /* Use a while loop to prevent the SO_RCVTIMEO property of the socket from creating a false start time due to a timeout */
    while((n = recvfrom(sockfd, &buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *)&cliaddr, &cliaddr_len) == -1));
    buffer[n] = '\0';
    count++;
    high_start = clock();
    for(int i = 0; i < packets-1; i++) {
        n = recvfrom(sockfd, &buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *)&cliaddr, &cliaddr_len);
        if(n >= 0) {
            buffer[n] = '\0';
            count++;
            high_end = clock();
        }
        else if(i == 0 && n == -1) {
            // Timeout is irrelevant in the inter measurement time.
            continue;
        }
        else break;
    }
    printf("Received: %d\n", count);

    high_time = (high_end - high_start) / (double) CLOCKS_PER_SEC;
    *low_arrival = low_time;
    *high_arrival = high_time;
    printf("High entropy arrival time: %lf sec\n", high_time);
    printf("first packet, last packet: %ld %ld\n", high_start, high_end);

    // informing the client that server is done with receiving and calculating
    char* message = "I am done.\n";
    sendto(sockfd, (const char *)message, strlen(message), 
    MSG_CONFIRM, (const struct sockaddr *) &cliaddr, cliaddr_len); 
    printf("Verification message was sent.\n");  

    close(sockfd);
    return 0;
}