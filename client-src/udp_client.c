#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include "../shared-src/structs.h"

// The packet length
#define PCKT_LEN 8192
#define MAX 1024

// total udp header length: 8 bytes (=64 bits)
// Function for checksum calculation. From the RFC,
// the checksum algorithm is:
//  "The checksum field is the 16 bit one's complement of the one's
//  complement sum of all 16 bit words in the header.  For purposes of
//  computing the checksum, the value of the checksum field is zero."
unsigned short csum(unsigned short *buf, int nwords) {
    unsigned long sum;
    for(sum = 0; nwords > 0; nwords--) {
        sum += *buf++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return (unsigned short)(~sum);
}

int udp_train(struct ini_info* info) {
    printf("We got to udp_train()!\n");
    int sockfd;

    // No data/payload just datagram
    char buffer[PCKT_LEN];

    char data[info->payload_size];

    // Our own headers' structures
    struct ipheader *ip = (struct ipheader *) buffer;
    struct udpheader *udp = (struct udpheader *)(buffer + sizeof(struct ipheader));


    // Source and destination addresses: IP and port
    struct sockaddr_in source_socket_in, dest_socket_in;
    int one = 1;
    const int *val = &one;

    memset(buffer, 0, PCKT_LEN);

    // Create a raw socket with UDP protocol
    sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if(sockfd < 0){
        perror("socket() error");
        exit(-1);
    } else {
        printf("socket() - Using SOCK_RAW socket and UDP protocol is OK.\n");
    }

    // Address families 
    source_socket_in.sin_family = AF_INET;
    dest_socket_in.sin_family = AF_INET;

    // Port numbers
    source_socket_in.sin_port = htons(info->train_udp.udph_srcport);
    dest_socket_in.sin_port = htons(info->train_udp.udph_destport);

    // IP addresses
    source_socket_in.sin_addr.s_addr = inet_addr("127.0.0.1");
    dest_socket_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Fabricate the IP header or we can use the
    // standard header structures but assign our own values.
    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip->iph_tos = 16; // Low delay
    ip->iph_len = sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(data);
    ip->iph_ident = htons(info->server_port);
    ip->iph_ttl = 1; // hops
    ip->iph_protocol = 17; // UDP

    // Source IP address, can use spoofed address here!!!
    ip->iph_sourceip = inet_addr("127.0.0.1");

    // The destination IP address
    ip->iph_destip = inet_addr("127.0.0.1");

    // Fabricate the UDP header. Source port number, redundant
    udp->udph_srcport = htons(info->train_udp.udph_srcport);

    // Destination port number
    udp->udph_destport = htons(info->train_udp.udph_destport);
    udp->udph_len = htons(sizeof(struct udpheader));

    // Calculate the checksum for integrity
    ip->iph_chksum = csum((unsigned short *) buffer, sizeof(struct ipheader) + sizeof(struct udpheader));

    // Inform the kernel do not fill up the packet structure. we will build our own...
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("setsockopt() error");
        exit(-1);
    } else {
        printf("setsockopt() is OK.\n");
    }
    printf("Using raw socket and UDP protocol\n");
    //int count;
    // Verify
    char buff[] = "Hello World";
    if(sendto(sockfd, buff, ip->iph_len, 0, (struct sockaddr *) & source_socket_in, sizeof(source_socket_in)) < 0) {
        perror("sendto() error");
        exit(-1);
    } else {
        printf("sendto() is OK.\n");
        sleep(2);
    }
    
    close(sockfd);

    return 0;
}
