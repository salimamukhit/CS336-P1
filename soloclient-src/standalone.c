#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
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
#include "../shared-src/msleep.h"
#include "../shared-src/logger.h"
#include "create_hdrs.h"
#include "sniff_rst.h"
#include "send_tcp.h"

#define _GNU_SOURCE
#define INI_NAME "solo_config.ini"
#define IP4_HDRLEN 20 // IPv4 header length

#define HEAD_TCP_NO 0
#define TAIL_TCP_NO 1

#define TCP_TYPE_NO 0
#define UDP_TYPE_NO 1

int send_udp(struct ini_info *info, int type);
void fillTrain(unsigned char** train, unsigned short int num, unsigned int size, int type);

/**
 * @brief takes the name of config file and performs standalone compression detection
 * 
 * @param argc number of arguments
 * @param argv the argument values (such as name of config file)
 * @return int 
 */
int main(int argc, char **argv) {
    /* Parse our INI */
    struct ini_info *info;
    if(argc == 1) {
        info = calloc(1, sizeof(struct ini_info));
        strcpy(info->file_name, INI_NAME);
    } else if(argc == 2) {
        info = calloc(1, sizeof(argv[1]));
        strcpy(info->file_name, argv[1]);
    } else {
        printf("Too many arguments!\n");
        exit(EXIT_FAILURE);
    }
    
    parse_ini(info);

    /* Initializing timing variables */
    clock_t low_start, low_end;
    clock_t high_start, high_end;
    double low_arrival, high_arrival;

    /* Send first TCP packet and collect RST packet */
    send_tcp(info, HEAD_TCP_NO);
    LOGP("HEAD SYN FOR LOW ENTROPY TRAIN\n");
    low_start = clock();

    /* Send first packet train*/
    send_udp(info, 0);
    LOGP("SENT UDP TRAIN #1\n");

    /* Send second TCP packet and collect RST packet */
    send_tcp(info, TAIL_TCP_NO);
    LOGP("TAIL SYN FOR LOW ENTROPY TRAIN\n");
    low_end = clock();
    low_arrival = (low_end - low_start) / (double) CLOCKS_PER_SEC;

    /* Waiting inter measurement time from configuration */
    sleep(info->meas_time);
    
    /* Repeating same steps for high entropy train */
    send_tcp(info, HEAD_TCP_NO);
    LOGP("HEAD SYN FOR HIGH ENTROPY TRAIN\n");
    high_start = clock();

    send_udp(info, 1);
    LOGP("SENT UDP TRAIN #2\n");

    send_tcp(info, TAIL_TCP_NO);
    LOGP("TAIL SYN FOR HIGH ENTROPY TRAIN\n");
    high_end = clock();
    high_arrival = (high_end - high_start) / (double) CLOCKS_PER_SEC;

    /* Printing out the results */
    printf("Low entropy train arrival time: %lf sec\n", low_arrival);
    printf("High entropy train arrival time: %lf sec\n", high_arrival);

    if((high_arrival - low_arrival) >= 0.1) {
        printf("Compression was detected.\n");
    } else {
        printf("No compression was detected\n");
    }

    return 0;
}

/**
 * @brief Sends UDP packets according to INI spec.
 * 
 * @param info The INI struct.
 * @param entropy_type 0 for low and 1 for high.
 * @return 0 for success and -1 for failure
 */
int send_udp(struct ini_info *info, int entropy_type) {
    int i, status, datalen, packet_len, sd, bytes, *ip_flags;
    char *interface, *target, *src_ip, *dst_ip;
    unsigned char *data;
    struct ip iphdr;
    struct udphdr udphdr;
    uint8_t *src_mac, *dst_mac, *udp_packet;
    struct addrinfo hints, *res;
    struct sockaddr_in *ipv4;
    struct sockaddr_ll device;
    struct ifreq ifr;
    void *tmp;
    unsigned char** train;

    // Allocate memory for various arrays.
    src_mac = calloc(6, sizeof(char));
    dst_mac = calloc(6, sizeof(char));
    data = calloc(info->payload_size, sizeof(unsigned char));
    udp_packet = calloc(IP_MAXPACKET, sizeof(unsigned char));
    interface = calloc(40, sizeof(unsigned char));
    target = calloc(40, sizeof(char));
    src_ip = calloc(INET_ADDRSTRLEN, sizeof(char));
    dst_ip = calloc(INET_ADDRSTRLEN, sizeof(char));
    ip_flags = calloc(4, sizeof(int));
    train = calloc(info->packet_num, sizeof(unsigned char*));

    memset(&udphdr, 0, sizeof(udphdr));

    for(i = 0; i < info->packet_num; i++) {
        *(train + i) = calloc(info->payload_size, sizeof(unsigned char));
    }

    // Fill Trains

    fillTrain(train, info->packet_num, info->payload_size, entropy_type);
    // Interface to send packet through.
    strcpy(interface, info->interface);

    // Submit request for a socket descriptor to look up interface.
    if((sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket() failed to get socket descriptor for using ioctl() ");
        exit(EXIT_FAILURE);
    }

    // Use ioctl() to look up interface name and get its MAC address.
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);
    if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0) {
        perror("ioctl() failed to get source MAC address ");
        return (EXIT_FAILURE);
    }
    close(sd);

    // Find interface index from interface name and store index in
    // struct sockaddr_ll device, which will be used as an argument of sendto().
    memset(&device, 0, sizeof(device));
    if((device.sll_ifindex = if_nametoindex(interface)) == 0) {
        perror("if_nametoindex() failed to obtain interface index ");
        exit(EXIT_FAILURE);
    }

    // Set destination MAC address: you need to fill this out
    dst_mac[0] = 0xff;
    dst_mac[1] = 0xff;
    dst_mac[2] = 0xff;
    dst_mac[3] = 0xff;
    dst_mac[4] = 0xff;
    dst_mac[5] = 0xff;

    // Source IPv4 address: you need to fill this out
    strcpy(src_ip, info->client_ip);

    // Destination URL or IPv4 address: you need to fill this out
    strcpy(target, info->standalone_dst);

    // Fill out hints for getaddrinfo().
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = hints.ai_flags | AI_CANONNAME;

    // Resolve target using getaddrinfo().
    if((status = getaddrinfo(target, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    ipv4 = (struct sockaddr_in *) res->ai_addr;
    tmp = &(ipv4->sin_addr);
    if(inet_ntop(AF_INET, tmp, dst_ip, INET_ADDRSTRLEN) == NULL) {
        status = errno;
        fprintf(stderr, "inet_ntop() failed.\nError message: %s", strerror(status));
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    // Fill out sockaddr_ll.
    device.sll_family = AF_PACKET;
    device.sll_protocol = htons(ETH_P_IP);
    memcpy(device.sll_addr, dst_mac, 6);
    device.sll_halen = 6;

    // UDP data
    datalen = info->payload_size;

    // IPv4 header
    iphdr.ip_sum = 0x00;
    create_ipheader(&iphdr, info, info->packet_ttl, UDP_TYPE_NO);

    // Source IPv4 address (32 bits)
    if((status = inet_pton(AF_INET, src_ip, &(iphdr.ip_src))) != 1) {
        fprintf(stderr, "inet_pton() failed.\nError message: %s", strerror(status));
        exit(EXIT_FAILURE);
    }

    // Destination IPv4 address(32 bits)
    if((status = inet_pton(AF_INET, dst_ip, &(iphdr.ip_dst))) != 1) {
        fprintf(stderr, "inet_pton() failed.\nError message: %s", strerror(status));
        exit(EXIT_FAILURE);
    }

    // UDP header
    create_udpheader(&iphdr, &udphdr, info, data);

    // Open raw socket descriptor.
    if((sd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL))) < 0) {
        perror("socket() failed ");
        exit(EXIT_FAILURE);
    }

    // Fill out ethernet frame header.
    // Ethernet frame length = ethernet data (IP header + UDP header + UDP data)
    packet_len = IP4_HDRLEN + UDP_HDRLEN + datalen;

    /* Hacky fix to fix the udp length */
    udphdr.len = htons(datalen + UDP_HDRLEN);

    for(i = 0; i < info->packet_num; i++) {
        iphdr.ip_id = htons(i);
        iphdr.ip_sum = 0x00;
        iphdr.ip_sum = checksum((uint16_t *) &iphdr, IP4_HDRLEN);
        memcpy(data, train[i], info->payload_size);
        memcpy(udp_packet, &iphdr, IP4_HDRLEN);
        memcpy(udp_packet + IP4_HDRLEN, &udphdr, UDP_HDRLEN);
        memcpy(udp_packet + IP4_HDRLEN + UDP_HDRLEN, data, datalen); // UDP data
        
        if((bytes = sendto(sd, udp_packet, packet_len, 0,(struct sockaddr *) &device, sizeof(device))) <= 0) {
            perror("sendto() failed");
            exit(EXIT_FAILURE);
        }
        memset(udp_packet, 0, packet_len);
    }

    close(sd);

    free(src_mac);
    free(dst_mac);
    free(data);
    free(udp_packet);
    free(interface);
    free(target);
    free(src_ip);
    free(dst_ip);
    free(ip_flags);
    for(i = 0; i < info->packet_num; i++) {
        free(train[i]);
    }
    free(train);

    return 0;
}

/**
 * @brief fills the packet train
 * 
 * @param char pointer to a train that can be filled with either low or high entropy data
 * @param num total number of packets in the train
 * @param size payload size of one packet
 * @param type 0 for low entropy train and 1 for high entropy train
 */
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
    else if(type == 1) {
        for(int i =0 ; i < num; i++) {
            for(int j = 0; j < size; j++) {
                train[i][j] = (unsigned char) rand();
            }
            train[i][0] = id >> 8;
            train[i][1] = id & mask_right;
            id += 0b1;
        }
    }
    else {
        perror("Invalid type for filling the packet train!\n");
    }
}