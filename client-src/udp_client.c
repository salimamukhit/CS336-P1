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

#include "../shared-src/logger.h"
#include "../shared-src/structs.h"
#include "../shared-src/msleep.h"

#define PORT 8080

void fillTrain(char** train, unsigned short int num, unsigned int size, int type) {
    unsigned char low_byte;
    unsigned char high_byte;

    if(type == 0) {
        for(unsigned int i=0; i<num; i++) {
            char *ptr = *(train+i);
            for(int j=0; j<size; j++) {
                *ptr = '0';
                ptr++;
            }
        }
    }
    else {
        FILE *fd = fopen("/dev/urandom", "r");
        for(int i=0; i<num; i++) {
            // Shift the start of the train over two bytes
            fgets((train[i] + 16), size, fd);
        }
        fclose(fd);
    }
}

int udp_train(struct ini_info* info) {
    printf("UDP client started\n");
    LOG("The port is: %d\n", htons(PORT));
    LOG("Inter-measurement time: %ld\n", info->meas_time);

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

    sleep(1); // giving server some time to finish binding
    sendto(sockfd, (const char *)message, strlen(message), MSG_CONFIRM, 
    (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Message was sent: %s\n", message);

    int packets = info->packet_num;
    //int packets = 10; //testing
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
        msleep(1);
        sendto(sockfd, low_entropy[i], size, MSG_CONFIRM, 
        (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }

    sleep(info->meas_time);

    for(int i = 0; i < packets; i++) {
        msleep(1);
        sendto(sockfd, high_entropy[i], size, MSG_CONFIRM, 
        (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }

    for(int i = 0; i < packets; i++) {
        free(low_entropy[i]);
        free(high_entropy[i]);
    }
    free(low_entropy);
    free(high_entropy);

    char buffer[1024] = { 0 };

    socklen_t len = (socklen_t) sizeof(servaddr);

    // the client is informed that server is done with receiving packets
    int m = recvfrom(sockfd, buffer, 10, MSG_WAITALL, (struct sockaddr* )&servaddr, (socklen_t *) &len);
    buffer[m] = '\0';
    printf("Received message from server: %s\n", buffer);

    close(sockfd);
    return 0;
}
