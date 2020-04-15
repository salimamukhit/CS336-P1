/**
* @file Handles the creation of the IP header.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, IPPROTO_IP, IPPROTO_TCP, INET_ADDRSTRLEN
#include <netinet/udp.h>
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#define __FAVOR_BSD           // Use BSD format of tcp header
#include <netinet/tcp.h>      // struct tcphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq

#include <errno.h>            // errno, perror()

#include "../shared-src/structs.h"
#include "create_hdrs.h"

// Define some constants.
#define IP4_HDRLEN 20         // IPv4 header length
#define TCP_HDRLEN 20         // TCP header length, excludes options data
#define UDP_HDRLEN 8
#define TCP_WINDOW_SIZE 65535

/* Pseudoheader (Used to compute TCP checksum. Check RFC 793) */
typedef struct tcp_pseudoheader {
    u_int32_t src;
    u_int32_t dst;
    unsigned char zero;
    unsigned char protocol;
    u_int16_t tcplen;
} tcp_phdr_t;

/* Pseudoheader (Used to compute UDP checksum) */
typedef struct udp_pseudoheader {
    u_int32_t src;
    u_int32_t dst;
    u_int16_t udplen;
} udp_phdr_t;

/**
 * @brief Computes the internet checksum (RFC 1071) for the IP Header.
 * 
 * @param addr the struct containing the address information.
 * @param len the length of the header.
 * @return uint16_t the checksum.
 */
uint16_t checksum(uint16_t *addr, int len) {
    int count = len;
    register uint32_t sum = 0;
    uint16_t answer = 0;

    // Sum up 2-byte values until none or only one byte left.
    while(count > 1) {
        sum += *(addr++);
        count -= 2;
    }

    // Add left-over byte, if any.
    if(count > 0) {
        sum += *(uint8_t *) addr;
    }

    // Fold 32-bit sum into 16 bits; we lose information by doing this,
    // increasing the chances of a collision.
    // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
    while(sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    // Checksum is one's compliment of sum.
    answer = ~sum;

    return answer;
}

// Build IPv4 TCP pseudo-header and call checksum function.
uint16_t tcp4_checksum(struct ip *iphdr, struct tcphdr *tcphdr) {
    /* TPC Pseudoheader (used in checksum)    */
    tcp_phdr_t pseudohdr;            

    /* TCP Pseudoheader + TCP actual header used for computing the checksum */
    char tcpcsumblock[sizeof(tcp_phdr_t) + TCP_HDRLEN];

    /* Fill the pseudoheader so we can compute the TCP checksum*/
    pseudohdr.src = iphdr->ip_src.s_addr;
    pseudohdr.dst = iphdr->ip_dst.s_addr;
    pseudohdr.zero = 0;
    pseudohdr.protocol = iphdr->ip_p;
    pseudohdr.tcplen = htons(sizeof(struct tcphdr));

    /* Copy header and pseudoheader to a buffer to compute the checksum */  
    memcpy(tcpcsumblock, &pseudohdr, sizeof(tcp_phdr_t));   
    memcpy((tcpcsumblock + sizeof(tcp_phdr_t)), tcphdr, sizeof(struct tcphdr));
        
    /* Compute the TCP checksum as the standard says (RFC 793) */
    tcphdr->th_sum = checksum((unsigned short *)(tcpcsumblock), sizeof(tcpcsumblock));

    return 0;
}

uint16_t udp4_checksum(struct ip *iphdr, struct udphdr *udphdr, uint8_t *payload, int payloadlen) {
    /* UDP Pseudoheader (used in checksum)    */
    udp_phdr_t pseudohdr;

    /* UDP Pseudoheader + UDP actual header used for computing the checksum */
    char udpcsumblock[sizeof(udp_phdr_t) + UDP_HDRLEN];

    /* Fill the pseudoheader so we can compute the UDP checksum*/
    pseudohdr.src = iphdr->ip_src.s_addr;
    pseudohdr.dst = iphdr->ip_dst.s_addr;
    pseudohdr.udplen = htons(sizeof(struct udphdr));

    /* Copy header and pseudoheader to a buffer to compute the checksum */  
    memcpy(udpcsumblock, &pseudohdr, sizeof(udp_phdr_t));   
    memcpy((udpcsumblock + sizeof(udp_phdr_t)), udphdr, sizeof(struct udphdr));
        
    /* Compute the UDP checksum as the standard says */
    udphdr->len = checksum((unsigned short *)(udpcsumblock), sizeof(udpcsumblock));

    return 0;
}

/**
 * @brief Define and create a IP Header for TCP/UDP.
 * 
 * @param iphdr the ip header to be filled.
 * @param info the parsed INI.
 * @param ttl the time to live for the packet.
 * @return int 0 for success and -1 for failure.
 */
int create_ipheader(struct ip *iphdr, struct ini_info *info, u_int8_t ttl) {
    int ip_flags[4] = { 0 };

    // IPv4 header length (4 bits): Number of 32-bit words in header = 5
    iphdr->ip_hl = 5; //IP4_HDRLEN / sizeof(uint32_t);

    // Internet Protocol version (4 bits): IPv4
    iphdr->ip_v = 4;

    // Type of service (8 bits)
    iphdr->ip_tos = 0;

    // Total length of datagram (16 bits): IP header + TCP header
    iphdr->ip_len = htons(IP4_HDRLEN + TCP_HDRLEN);

    // ID sequence number (16 bits): unused, since single datagram
    iphdr->ip_id = htons(0);

    // Flags, and Fragmentation offset (3, 13 bits): 0 since single datagram

    // Zero (1 bit)
    ip_flags[0] = 0;

    // Do not fragment flag (1 bit)
    ip_flags[1] = 1;

    // More fragments following flag (1 bit)
    ip_flags[2] = 0;

    // Fragmentation offset (13 bits)
    ip_flags[3] = 0;

    iphdr->ip_off = htons ((ip_flags[0] << 15)
                        + (ip_flags[1] << 14)
                        + (ip_flags[2] << 13)
                        +  ip_flags[3]);

    // Time-to-Live (8 bits): default to maximum value
    iphdr->ip_ttl = ttl;

    // Transport layer protocol (8 bits): 6 for TCP
    iphdr->ip_p = IPPROTO_TCP;

    int status;
    // Source IPv4 address (32 bits)
    if((status = inet_pton (AF_INET, info->client_ip, &(iphdr->ip_src))) != 1) {
        fprintf(stderr, "inet_pton() failed.\nError message: %s", strerror(status));
        exit(EXIT_FAILURE);
    }

    // Destination IPv4 address (32 bits)
    if ((status = inet_pton (AF_INET, info->standalone_dst, &(iphdr->ip_dst))) != 1) {
        fprintf(stderr, "inet_pton() failed.\nError message: %s", strerror(status));
        exit(EXIT_FAILURE);
    }

    // IPv4 header checksum (16 bits): set to 0 when calculating checksum
    iphdr->ip_sum = 0;
    iphdr->ip_sum = checksum((uint16_t *) &iphdr, IP4_HDRLEN);

    return 0;
}

/**
 * @brief Creates and fills out a TCP Header.
 * 
 * @param iphdr the filled out IP Header.
 * @param tcphdr the TCP header to be filled.
 * @param info the parsed INI.
 * @return int 0 for success and -1 for failure.
 */
int create_tcpheader(struct ip *iphdr, struct tcphdr *tcphdr, struct ini_info *info) {
    int tcp_flags[8] = { 0 };

    // Source port number (16 bits)
    tcphdr->th_sport = htons(info->server_port);  // 60

    // Destination port number (16 bits)
    tcphdr->th_dport = htons(info->head_port);  //80

    // Sequence number (32 bits)
    tcphdr->th_seq = htonl(0);

    // Acknowledgement number (32 bits): 0 in first packet of SYN/ACK process
    tcphdr->th_ack = htonl(0);

    // Reserved (4 bits): should be 0
    tcphdr->th_x2 = 0;
    
    // Data offset (4 bits): size of TCP header in 32-bit words
    tcphdr->th_off = TCP_HDRLEN / sizeof (uint32_t);

    // Flags (8 bits)

    // FIN flag (1 bit)
    tcp_flags[0] = 0;

    // SYN flag (1 bit): set to 1
    tcp_flags[1] = 1;

    // RST flag (1 bit)
    tcp_flags[2] = 0;

    // PSH flag (1 bit)
    tcp_flags[3] = 0;

    // ACK flag (1 bit)
    tcp_flags[4] = 0;

    // URG flag (1 bit)
    tcp_flags[5] = 0;

    // ECE flag (1 bit)
    tcp_flags[6] = 0;

    // CWR flag (1 bit)
    tcp_flags[7] = 0;

    tcphdr->th_flags = 0;
    for(int i = 0; i < 8; i++) {
        tcphdr->th_flags += (tcp_flags[i] << i);
    }

    // Window size (16 bits)
    tcphdr->th_win = htons(TCP_WINDOW_SIZE);

    // Urgent pointer (16 bits): 0 (only valid if URG flag is set)
    tcphdr->th_urp = htons(0);

    // TCP checksum (16 bits): set to zero until calculated.
    tcphdr->th_sum = 0;

    tcp4_checksum(iphdr, tcphdr);

    return 0;
}

/**
 * @brief Creates and fills out a UDP header.
 * 
 * @param ipheader pointer to an ip header with which udp header is associated
 * @param udpheader pointer to a udp header to be filled
 * @param info configurations
 * @param data payload
 * @return 0 which is success
 */
int create_udpheader(struct ip* iphdr, struct udphdr* udpheader, struct ini_info* info, unsigned char *data) {
    udpheader->source = htons(info->train_udp.udph_srcport);
    udpheader->dest = htons(info->train_udp.udph_destport);
    udpheader->len = info->payload_size + UDP_HDRLEN;
    udpheader->check = udp4_checksum(iphdr, udpheader, data, info->payload_size);
    return 0;
}