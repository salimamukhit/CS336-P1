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
    uint16_t svalue;
    char buf[IP_MAXPACKET], cvalue;
    char *ptr;
    int chksumlen = 0;

    // ptr points to beginning of buffer buf
    ptr = &buf[0];

    // Copy source IP address into buf (32 bits)
    memcpy (ptr, &iphdr->ip_src.s_addr, sizeof (iphdr->ip_src.s_addr));
    ptr += sizeof (iphdr->ip_src.s_addr);
    chksumlen += sizeof (iphdr->ip_src.s_addr);

    // Copy destination IP address into buf (32 bits)
    memcpy (ptr, &iphdr->ip_dst.s_addr, sizeof (iphdr->ip_dst.s_addr));
    ptr += sizeof (iphdr->ip_dst.s_addr);
    chksumlen += sizeof (iphdr->ip_dst.s_addr);

    // Copy zero field to buf (8 bits)
    *ptr = 0; ptr++;
    chksumlen += 1;

    // Copy transport layer protocol to buf (8 bits)
    memcpy (ptr, &iphdr->ip_p, sizeof (iphdr->ip_p));
    ptr += sizeof (iphdr->ip_p);
    chksumlen += sizeof (iphdr->ip_p);

    // Copy TCP length to buf (16 bits)
    svalue = htons (sizeof (tcphdr));
    memcpy (ptr, &svalue, sizeof (svalue));
    ptr += sizeof (svalue);
    chksumlen += sizeof (svalue);

    // Copy TCP source port to buf (16 bits)
    memcpy (ptr, &tcphdr->th_sport, sizeof (tcphdr->th_sport));
    ptr += sizeof (tcphdr->th_sport);
    chksumlen += sizeof (tcphdr->th_sport);

    // Copy TCP destination port to buf (16 bits)
    memcpy (ptr, &tcphdr->th_dport, sizeof (tcphdr->th_dport));
    ptr += sizeof (tcphdr->th_dport);
    chksumlen += sizeof (tcphdr->th_dport);

    // Copy sequence number to buf (32 bits)
    memcpy (ptr, &tcphdr->th_seq, sizeof (tcphdr->th_seq));
    ptr += sizeof (tcphdr->th_seq);
    chksumlen += sizeof (tcphdr->th_seq);

    // Copy acknowledgement number to buf (32 bits)
    memcpy (ptr, &tcphdr->th_ack, sizeof (tcphdr->th_ack));
    ptr += sizeof (tcphdr->th_ack);
    chksumlen += sizeof (tcphdr->th_ack);

    // Copy data offset to buf (4 bits) and
    // copy reserved bits to buf (4 bits)
    cvalue = (tcphdr->th_off << 4) + tcphdr->th_x2;
    memcpy (ptr, &cvalue, sizeof (cvalue));
    ptr += sizeof (cvalue);
    chksumlen += sizeof (cvalue);

    // Copy TCP flags to buf (8 bits)
    memcpy (ptr, &tcphdr->th_flags, sizeof (tcphdr->th_flags));
    ptr += sizeof (tcphdr->th_flags);
    chksumlen += sizeof (tcphdr->th_flags);

    // Copy TCP window size to buf (16 bits)
    memcpy (ptr, &tcphdr->th_win, sizeof (tcphdr->th_win));
    ptr += sizeof (tcphdr->th_win);
    chksumlen += sizeof (tcphdr->th_win);

    // Copy TCP checksum to buf (16 bits)
    // Zero, since we don't know it yet
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    chksumlen += 2;

    // Copy urgent pointer to buf (16 bits)
    memcpy (ptr, &tcphdr->th_urp, sizeof (tcphdr->th_urp));
    ptr += sizeof (tcphdr->th_urp);
    chksumlen += sizeof (tcphdr->th_urp);

    return checksum ((uint16_t *) buf, chksumlen);
}

uint16_t udp4_checksum (struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen) {
    char buf[IP_MAXPACKET];
    char *ptr;
    int chksumlen = 0;
    int i;

    ptr = &buf[0];  // ptr points to beginning of buffer buf

    // Copy source IP address into buf (32 bits)

    memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
    ptr += sizeof (iphdr.ip_src.s_addr);
    chksumlen += sizeof (iphdr.ip_src.s_addr);

    // Copy destination IP address into buf (32 bits)
    memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
    ptr += sizeof (iphdr.ip_dst.s_addr);
    chksumlen += sizeof (iphdr.ip_dst.s_addr);

    // Copy zero field to buf (8 bits)
    *ptr = 0; ptr++;
    chksumlen += 1;

    // Copy transport layer protocol to buf (8 bits)
    memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
    ptr += sizeof (iphdr.ip_p);
    chksumlen += sizeof (iphdr.ip_p);

    // Copy UDP length to buf (16 bits)
    memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
    ptr += sizeof (udphdr.len);
    chksumlen += sizeof (udphdr.len);

    // Copy UDP source port to buf (16 bits)
    memcpy (ptr, &udphdr.source, sizeof (udphdr.source));
    ptr += sizeof (udphdr.source);
    chksumlen += sizeof (udphdr.source);

    // Copy UDP destination port to buf (16 bits)
    memcpy (ptr, &udphdr.dest, sizeof (udphdr.dest));
    ptr += sizeof (udphdr.dest);
    chksumlen += sizeof (udphdr.dest);

    // Copy UDP length again to buf (16 bits)
    memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
    ptr += sizeof (udphdr.len);
    chksumlen += sizeof (udphdr.len);

    // Copy UDP checksum to buf (16 bits)
    // Zero, since we don't know it yet
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    chksumlen += 2;

    // Copy payload to buf
    memcpy (ptr, payload, payloadlen);
    ptr += payloadlen;
    chksumlen += payloadlen;

    // Pad to the next 16-bit boundary
    for (i = 0; i < payloadlen%2; i++, ptr++) {
        *ptr = 0;
        ptr++;
        chksumlen++;
    }
    return checksum ((uint16_t *) buf, chksumlen);
}

/**
 * @brief Define and create a IP Header for TCP/UDP.
 * 
 * @param iphdr the ip header to be filled.
 * @param info the parsed INI.
 * @param ttl the time to live for the packet.
 * @return int 0 for success and -1 for failure.
 */
int create_ipheader(struct ip *iphdr, struct ini_info *info, unsigned int ttl) {
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

    //char *dst_ip = "127.0.0.1";  // "1.1.1.1";  //"107.180.95.33"; // VPS IP
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
    tcphdr->th_off = TCP_HDRLEN / 4;

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

    // TCP checksum (16 bits)
    tcphdr->th_sum = tcp4_checksum(iphdr, tcphdr);

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
int create_udpheader(struct ip* iphdr, struct udphdr* udpheader, struct ini_info* info, char* data) {
    udpheader->source = htons(info->train_udp.udph_srcport);
    udpheader->dest = htons(info->train_udp.udph_destport);
    udpheader->len = info->payload_size + UDP_HDRLEN;
    udpheader->check = udp4_checksum(iphdr, udpheader, data, info->payload_size);
    return 0;
}