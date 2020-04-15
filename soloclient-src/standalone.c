#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <pcap.h>

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

#include "create_hdrs.h"

#define _GNU_SOURCE
#define INI_NAME "solo_config.ini"
#define MAX_CAPTURE_BYTES 2048

void set_ifr(struct ifreq *ifr, int *sockfd, char *interface_name);
int get_rst(struct ini_info *info);

/**
 * @brief This function helps set the ifreq for the networking interface.
 * 
 *        Set the name in ifr.ifr_name and then use ioctl([socket], SIOCGIFINDEX, &ifr)
 *        to set the index of the networking device. This sets ifr.ifr_ifindex;
 * 
 *        This in particular is used for the setsockopt() function to specify the hardware interface.
 * 
 * @param sockfd the unopened socket file descriptor. (Use in the beginning before setting ip header).
 */
void set_ifr(struct ifreq *ifr, int *sockfd, char *interface_name) {

    // Submit request for a socket descriptor to look up interface.
    if((*sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket() failed to get socket descriptor for using ioctl() ");
        exit (EXIT_FAILURE);
    }

    /* Use ioctl() to look up interface index which we will use to
    ** bind socket descriptor sd to specified interface with setsockopt() since
    ** none of the other arguments of sendto() specify which interface to use.
    */
    snprintf(ifr->ifr_ifrn.ifrn_name, strlen(interface_name) + 1, "%s", interface_name);
    if(ioctl(*sockfd, SIOCGIFINDEX, ifr) < 0) {
        printf("Device: '%s' | '%s'\n", interface_name, ifr->ifr_ifrn.ifrn_name);
        perror("ioctl() failed to find interface ");
        exit(EXIT_FAILURE);
    }

    close(*sockfd);
    printf("Index for interface %s is %i\n", interface_name, ifr->ifr_ifindex);
}

int main(int argc, char **argv) {
    /* Parse our INI */
    struct ini_info *info = calloc(1, sizeof(struct ini_info));
    strcpy(info->file_name, INI_NAME);
    parse_ini(info);

    /* Error Status Tracking */
    int status;

    /* Declare descritor for the interface */
    struct ifreq *ifr = calloc(1, sizeof(ifr));

    /* File descritor for the socket */
    int sockfd;

    /* Define Flags */
    int *ip_flags;
    int *tcp_flags;

    /* Set to toggle options in setsockopt() on or off */
    const int on = 1;

    /* Definition of IP/TCP headers */
    struct ip iphdr;
    struct tcphdr tcphdr;
    struct addrinfo *resolved_target;
    struct sockaddr_in *ipv4;
    struct sockaddr_in sin;  // = calloc(1, sizeof(struct sockaddr_in));

    /* Allocate/Set memory for IP headers, flags, hints and other structs */
    uint8_t *packet = calloc(IP_MAXPACKET, sizeof(uint8_t));
    struct addrinfo *hints = calloc(1, sizeof(struct addrinfo));
    ip_flags = calloc(4, sizeof(int));
    tcp_flags = calloc(8, sizeof(int));
    char *dst_ip = calloc(INET_ADDRSTRLEN, sizeof(char));




    /* ----------------- Initialization is done, now we create the IP Header ----------------- */

    /* We now need to get and set the interface name for the socket descriptor */
    set_ifr(ifr, &sockfd, info->interface);

    // Fill out hints for getaddrinfo().
    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = hints->ai_flags | AI_CANONNAME;

    // Resolve target using getaddrinfo().
    if((status = getaddrinfo(info->standalone_dst, NULL, hints, &resolved_target)) != 0) {
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    ipv4 = (struct sockaddr_in *) resolved_target->ai_addr;
    void *tmp = &(ipv4->sin_addr);
    if(inet_ntop(AF_INET, tmp, dst_ip, INET_ADDRSTRLEN) == NULL) {
        status = errno;
        fprintf(stderr, "inet_ntop() failed.\nError message: %s", strerror (status));
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(resolved_target);


    /* --- IPv4 Header Stage --- */
    
    create_ipheader(&iphdr, info, 255);

    /* --- TCP Header Stage --- */

    create_tcpheader(&iphdr, &tcphdr, info);


    /* --- Prepare the packet --- */

    // First part is an IPv4 header.
    memcpy(packet, &iphdr, IP4_HDRLEN * sizeof(uint8_t));

    // Next part of packet is upper layer protocol header.
    memcpy((packet + IP4_HDRLEN), &tcphdr, TCP_HDRLEN * sizeof(uint8_t));

    // The kernel is going to prepare layer 2 information (ethernet frame header) for us.
    // For that, we need to specify a destination for the kernel in order for it
    // to decide where to send the raw datagram. We fill in a struct in_addr with
    // the desired destination IP address, and pass this structure to the sendto() function.
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = iphdr.ip_dst.s_addr;

    // Submit request for a raw socket descriptor.
    if((sockfd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket() failed ");
        exit(EXIT_FAILURE);
    }

    /* We need to tell the kernel that we'll be adding our own IP header */
    // Set flag so socket expects us to provide IPv4 header.
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt() failed to set IP_HDRINCL ");
        exit(EXIT_FAILURE);
    }

    // Bind socket to interface index.
    if(setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifr, sizeof(*ifr)) < 0) {
        perror("setsockopt() failed to bind to interface ");
        exit(EXIT_FAILURE);
    }

    // Send packet.
    if(sendto(sockfd, packet, IP4_HDRLEN + TCP_HDRLEN, 0, (const struct sockaddr *) &sin, sizeof(struct sockaddr)) < 0)  {
        perror("sendto() failed ");
        exit(EXIT_FAILURE);
    }

    // Close socket descriptor.
    close(sockfd);

    // Wait for RST flag
    get_rst(info);


    return 0;
}

/**
 * @brief Uses libpcap to retrive the RST packet.
 * 
 * @param info the struct from the INI.
 * @return int 0 for success and -1 for failure.
 */
int get_rst(struct ini_info *info) {
    /* The var for the raw data of the packet */
    const unsigned char *packet = NULL;
    /* To Store network address and netmask */ 
    bpf_u_int32 netaddr=0, mask=0;
    /* Packet information (timestamp,size...) */ 
    struct pcap_pkthdr packet_header;
    /* Place to store the BPF filter program */
    struct bpf_program filter;
    /* Buffer for pcap errors */
    char errbuf[PCAP_ERRBUF_SIZE];
    /* The specified device interface from the INI */
    char *dev = info->interface;
    /* The device descriptor for libpcap */
    pcap_t *dev_descriptor = NULL;

    pcap_t *handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if(handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        exit(EXIT_FAILURE);
    }

    /* Look up info from the capture device. */ 
    if(pcap_lookupnet(dev, &netaddr, &mask, errbuf) == -1 ){ 
        fprintf(stderr, "ERROR: pcap_lookupnet(): %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    /* Compiles the filter expression into a BPF filter program */
    if(pcap_compile(dev_descriptor, &filter, "(tcp[13] == 0x10) or (tcp[13] == 0x18)", 1, mask) == -1){
        fprintf(stderr, "Error in pcap_compile(): %s\n", pcap_geterr(dev));
        exit(EXIT_FAILURE);
    }
    
    /* Load the filter program into the packet capture device. */ 
    if(pcap_setfilter(dev_descriptor, &filter) == -1 ){
        fprintf(stderr, "Error in pcap_setfilter(): %s\n", pcap_geterr(dev));
        exit(EXIT_FAILURE);
    }

    if((packet = pcap_next(dev_descriptor, &packet_header)) == NULL){
        fprintf(stderr, "Error in pcap_next()\n", errbuf);
        exit(EXIT_FAILURE);
    }


    struct ip *iphdr = (struct ip *)(packet+14);
    struct tcphdr *tcphdr = (struct tcphdr *)(packet+14+20);
    printf("+-------------------------+\n");
    printf("   ACK: %u\n", ntohl(tcphdr->th_ack) ); 
    printf("   SEQ: %u\n", ntohl(tcphdr->th_seq) );
    printf("   DST IP: %s\n", inet_ntoa(iphdr->ip_dst)); 
    printf("   SRC IP: %s\n", inet_ntoa(iphdr->ip_src)); 
    printf("   SRC PORT: %d\n", ntohs(tcphdr->th_sport) ); 
    printf("   DST PORT: %d\n", ntohs(tcphdr->th_dport) );
    printf("+-------------------------+\n");

    return 0;
}