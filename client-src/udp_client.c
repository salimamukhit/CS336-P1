#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

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
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

void fillTrain(unsigned char** train, unsigned short int num, unsigned int size, int type) {
    srand(time(NULL));
    u_int16_t id = 0x00;
    u_int16_t mask_right = 0b0000000011111111; // Or we can represent it as 0x0F

    if(type == 0) {
        for(int i =0 ; i < num; i++) {
            for(int j = 0; j < size; j++) {
                train[i][j] = 0x0;
            }
            train[i][0] = id >> 8;
            train[i][1] = id & mask_right;
            id += 0b1;
        }
    }
    else {
        for(int i =0 ; i < num; i++) {
            for(int j = 0; j < size; j++) {
                train[i][j] = (unsigned char) rand();
            }
            train[i][0] = id >> 8;
            train[i][1] = id & mask_right;
            id += 0b1;
        }
    }
}

void print_payload(unsigned char *payload, unsigned int size) {
    for(int i = 0; i < size; i++) {
        printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(payload[i]));
    }
    puts("\n--------------------------------------------------------");
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

    unsigned char** low_entropy = calloc(packets, sizeof(unsigned char*));
    unsigned char** high_entropy = calloc(packets, sizeof(unsigned char*));
    for(int i = 0; i < packets; i++) {
        low_entropy[i] = calloc(size, sizeof(unsigned char));
        high_entropy[i] = calloc(size, sizeof(unsigned char));
    }
    fillTrain(low_entropy, packets, size, 0);
    fillTrain(high_entropy, packets, size, 1);

    //printf("Low entropy packet:\n");
    //print_payload(low_entropy[0], size);
    //printf("High entropy packet:\n");
    //print_payload(high_entropy[0], size);

    //printf("Low entropy packet:  %.*s\n", packets, low_entropy[0]);
    puts("Low Entropy Data:");
    print_payload(*low_entropy, info->payload_size);
    puts("");
    //puts("High entropy gets null-terminated prematurely becuase of the randomness.");
    //printf("High entropy packet:  %s\n", high_entropy[0]);
    puts("High Entropy Data:");
    print_payload(*high_entropy, info->payload_size);
    puts("");

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
