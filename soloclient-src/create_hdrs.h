/**
* @file Handles the creation of the IP header.
*/

#ifndef _CREATE_HDRS_H_
#define _CREATE_HDRS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, IPPROTO_IP, IPPROTO_TCP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/tcp.h>      // struct tcphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq

#include <errno.h>            // errno, perror()

#include "../shared-src/structs.h"

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
uint16_t checksum(uint16_t *addr, int len);

/**
 * @brief Define and create a IP Header for TCP/UDP.
 * 
 * @param iphdr the ip header to be filled.
 * @param info the parsed INI.
 * @param ttl the time to live for the packet.
 * @return int 0 for success and -1 for failure.
 */
int create_ipheader(struct ip *iphdr, struct ini_info *info, u_int8_t ttl);

/**
 * @brief Creates and fills out a TCP Header.
 * 
 * @param iphdr the filled out IP Header.
 * @param tcphdr the TCP header to be filled.
 * @param info the parsed INI.
 * @return int 0 for success and -1 for failure.
 */
int create_tcpheader(struct ip *iphdr, struct tcphdr *tcphdr, struct ini_info *info);

int create_udpheader(struct ip* iphdr, struct udphdr* udpheader, struct ini_info* info, unsigned char *data);

#endif
