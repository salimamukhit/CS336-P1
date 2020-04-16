#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <pcap.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include "../shared-src/ini_parser.h"
#include "../shared-src/structs.h"
#include "../shared-src/logger.h"
#include "create_hdrs.h"
#include "sniff_rst.h"
#include "send_tcp.h"

#define _GNU_SOURCE
#define INI_NAME "solo_config.ini"
#define MAX_CAPTURE_BYTES 2048
#define HEAD_TCP_NO 0
#define TAIL_TCP_NO 1

int send_train(struct ini_info *info, struct udphdr *udphdr, struct ip *iphdr, int type, int *sockfd, uint8_t *ether_frame, struct sockaddr_ll *device);
void fillTrain(unsigned char** train, unsigned short int num, unsigned int size, int type);


int main(int argc, char **argv) {
    /* Parse our INI */
    struct ini_info *info = calloc(1, sizeof(struct ini_info));
    strcpy(info->file_name, INI_NAME);
    parse_ini(info);

    send_tcp(info, HEAD_TCP_NO);

    return 0;
}

int send_train(struct ini_info *info, struct udphdr *udphdr, struct ip* iphdr, int type, int *sockfd, uint8_t *ether_frame, struct sockaddr_ll *device) {
    int frame_length = IP4_HDRLEN + UDP_HDRLEN + info->payload_size;
    int bytes;

    /* Filling up the trains with data */
    unsigned char** low_train = (unsigned char**) calloc(info->payload_size, sizeof(unsigned char));
    unsigned char** high_train = (unsigned char**) calloc(info->payload_size, sizeof(unsigned char));
    fillTrain(low_train, info->packet_num, info->payload_size, 0);
    fillTrain(high_train, info->packet_num, info->payload_size, 1);

    create_udpheader(iphdr, udphdr, info, low_train[0]);

    // IPv4 header
    memcpy (ether_frame, &iphdr, IP4_HDRLEN);

    // UDP header
    memcpy (ether_frame + IP4_HDRLEN, udphdr, UDP_HDRLEN);

    // UDP data
    memcpy (ether_frame + IP4_HDRLEN + UDP_HDRLEN, low_train[0], info->payload_size);
    
    if((bytes = sendto (*sockfd, ether_frame, frame_length, 0, (struct sockaddr *) device, sizeof (device))) <= 0) {
        perror ("UDP sendto()");
        return -1;
    }
    return 0;
}

void fillTrain(unsigned char** train, unsigned short int num, unsigned int size, int type) {
    //unsigned char low_byte;
    //unsigned char high_byte;

    if(type == 0) {
        for(unsigned int i=0; i<num; i++) {
            unsigned char *ptr = *(train+i);
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
            fgets((char *) train[i], size, fd);
            train[i][0] = 'F';
            train[i][1] = 'G';
        }
        fclose(fd);
    }
}