#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#include "../shared-src/structs.h"

#define PORT 8080

// total udp header length: 8 bytes (=64 bits)
// Function for checksum calculation. From the RFC,
// the checksum algorithm is:
//  "The checksum field is the 16 bit one's complement of the one's
//  complement sum of all 16 bit words in the header.  For purposes of
//  computing the checksum, the value of the checksum field is zero."

void fillTrain(char** train, unsigned short int num, unsigned int size, int type) {
    if(type == 0) {
        for(int i=0; i<num; i++) {
            char *ptr = *(train+i);
            for(int j=0; j<size; j++) {
                *ptr = '0';
                ptr++;
            }
        }
    } else {
        FILE *fd = fopen("/dev/urandom", "r");
        for(int i=0; i<num; i++) {
            fgets(train[i], size, fd);
        }
        fclose(fd);
    }
}

int udp_train(struct ini_info* info) {
    printf("UDP client started\n");
    printf("The port is: %d\n", htons(PORT));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    char *message = "Hello Server!";
    struct sockaddr_in servaddr;

    if(sockfd < 0) {
        perror("Socket");
        return -1;
    }
    printf("Socket creation was successful!\n");

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = info->server_ip.s_addr; //INADDR_ANY;
    printf("Address is %d\n", info->server_ip.s_addr);

    sleep(1);
    sendto(sockfd, (const char *)message, strlen(message), MSG_CONFIRM, 
    (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Message was sent: %s\n", message);

    //int packets = info->packet_num;
    int packets = 2;
    int size = info->payload_size;

    char** low_entropy = calloc(packets, sizeof(char*));
    char** high_entropy = calloc(packets, sizeof(char*));
    for(int i = 0; i < packets; i++) {
        low_entropy[i] = calloc(size, sizeof(char));
        high_entropy[i] = calloc(size, sizeof(char));
    }
    fillTrain(low_entropy, packets, size, 0);
    fillTrain(high_entropy, packets, size, 1);

    printf("Low entropy packet: %s\n", low_entropy[0]);
    printf("High entropy packet: %s\n", high_entropy[0]);

    for(int i = 0; i < packets; i++) {
        sleep(1);
        sendto(sockfd, low_entropy[i], size, MSG_CONFIRM, 
        (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }

    sleep(info->meas_time);

    for(int i = 0; i < packets; i++) {
        sleep(1);
        sendto(sockfd, high_entropy[i], size, MSG_CONFIRM, 
        (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }

    for(int i = 0; i < packets; i++) {
        free(low_entropy[i]);
        free(high_entropy[i]);
    }
    free(low_entropy);
    free(high_entropy);

    close(sockfd);
    return 0;
}
