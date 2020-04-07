#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
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
#include "../client-src/udp_client.h"

#define INTERFACE "wlp7s0"
#define INI_NAME "config.ini"
#define MY_IP "127.0.0.1"
#define TARGET_IP "64.233.163.2" // google's IP
#define PCKT_LEN 8192

/* Checksum functions */
unsigned short csum(unsigned short *buf, int len);

int main() {
    // getting the mac address
    char* interface;
    struct ifreq ifr;
    uint8_t mac_src[6];
    
    printf("Starting the standalone compression detection application.\n");

    interface = calloc(1, sizeof(INTERFACE));
    strcpy(interface, INTERFACE);
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sockfd < 0) {
        perror("Socket");
        return EXIT_FAILURE;
    }
    printf("Socket creation was successful\n");

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);

    if(ioctl(sockfd, SIOCGIFHWADDR, & ifr) < 0) {
        perror("ioctl() failed to get source MAC address ");
        return EXIT_FAILURE;
    }
    close(sockfd);

    memcpy(mac_src, ifr.ifr_hwaddr.sa_data, 6);
    printf("MAC address for interface %s is ", interface);

    for(int i = 0; i < 5; i++) {
        printf("%02x:", mac_src[i]);
    }

    printf("%02x\n", mac_src[5]);
    free(interface);

    struct ini_info *info = calloc(1, sizeof(struct ini_info));
    strcpy(info->file_name, INI_NAME);
    if(parse_ini(info) != 0) {
        printf("Problem with INI file\n");
        exit(-1);
    }
    printf("Parsed the ini file\n");
    // creating tcp head and tail
    char buffer[PCKT_LEN];
    struct ipheader *ip = (struct ipheader *) buffer;
    struct tcpheader *tcp = (struct tcpheader *) (buffer + sizeof(struct ipheader));
    struct sockaddr_in sin, din;
    int one = 1;
    const int *val = &one;
    memset(buffer, 0, PCKT_LEN);
    int sd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
    if(sd < 0) {
        perror("socket() error");
        exit(-1);
    }
    printf("socket()-SOCK_RAW and tcp protocol is OK.\n");
    // The source is redundant, may be used later if needed
    // Address family
    sin.sin_family = AF_INET;
    din.sin_family = AF_INET;
    // Source port, can be any, modify as needed
    sin.sin_port = htons(info->server_port);
    din.sin_port = htons(info->head_port);
    // Source IP, can be any, modify as needed
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    din.sin_addr.s_addr = inet_addr("64.233.160.5");
    // IP structure
    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip->iph_tos = 16;
    ip->iph_len = sizeof(struct ipheader) + sizeof(struct tcpheader);
    ip->iph_ident = htons(54321);
    ip->iph_offset = 0;
    ip->iph_ttl = 64;
    ip->iph_protocol = 6; // TCP
    ip->iph_chksum = 0; // Done by kernel
    // Source IP, modify as needed, spoofed, we accept through command line argument
    ip->iph_sourceip = inet_addr("127.0.0.1");
    // Destination IP, modify as needed, but here we accept through command line argument
    ip->iph_destip = inet_addr("64.233.160.5");
    // The TCP structure. The source port, spoofed, we accept through the command line
    tcp->tcph_srcport = htons(info->server_port);
    // The destination port, we accept through command line
    tcp->tcph_destport = htons(info->head_port);
    tcp->tcph_seqnum = htonl(1);
    tcp->tcph_acknum = 0;
    tcp->tcph_offset = 5;
    tcp->tcph_syn = 1;
    tcp->tcph_ack = 0;
    tcp->tcph_win = htons(32767);
    tcp->tcph_chksum = 0; // Done by kernel
    tcp->tcph_urgptr = 0;
    // IP checksum calculation
    ip->iph_chksum = csum((unsigned short *) buffer, (sizeof(struct ipheader) + sizeof(struct tcpheader)));
    // Inform the kernel do not fill up the headers' structure, we fabricated our own
    if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("setsockopt() error");
        exit(-1);
    }
    printf("setsockopt() is OK\n");
    printf("Sending the head\n");
    if(sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("sendto() error");
        exit(-1);
    }
    printf("Packet was sent\n");
    sleep(2);
    tcp->tcph_destport = htons(info->tail_port);
    din.sin_port = htons(info->tail_port);
    printf("Sending the tail on port %d from %d\n", htons(info->tail_port), htons(info->server_port));
    if(sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("sendto() error");
        exit(-1);
    }
    printf("Packet was sent\n");
    
        
    close(sd);
    free(info);
    // creating udp packets
    // sending everything
    // receiving RST and ICMP packets
    return 0;
}

unsigned short csum(unsigned short *buf, int len) {
        unsigned long sum;
        for(sum=0; len>0; len--)
            sum += *buf++;
        sum = (sum >> 16) + (sum &0xffff);
        sum += (sum >> 16);
        return (unsigned short)(~sum);

}
