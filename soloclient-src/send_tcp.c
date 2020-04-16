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

#define TCP 0
#define HEAD 0
#define TAIL 1
#define WAIT_SERVER 10

void set_ifr(struct ifreq *ifr, int *sockfd, char *interface_name);

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
    // printf("Index for interface %s is %i\n", interface_name, ifr->ifr_ifindex);
}

/**
 * @brief Sends a TCP packet using RAW SOCKETS. This can only be used for head and tail packets.
 *        It also handles reciving the RST packet.
 * 
 * @param info the struct with the info from the INI.
 * @param packet_no either 0 for head or 1 for tail. Other values will break this.
 * @return int 0 on success and -1 on failure.
 */
int send_tcp(struct ini_info *info, unsigned int packet_no) {
    /* Error Status Tracking */
    int status;

    /* Declare descritor for the interface */
    struct ifreq *ifr = calloc(1, sizeof(ifr));

    /* File descritor for the socket */
    int sockfd; // for TCP

    /* Set to toggle options in setsockopt() on or off */
    const int on = 1;

    /* Definition of IP/TCP/UDP headers */
    struct ip iphdr;
    struct tcphdr tcphdr;
    struct addrinfo *resolved_target;
    struct sockaddr_in *ipv4;
    struct sockaddr_in sin;  // = calloc(1, sizeof(struct sockaddr_in));

    /* Allocate/Set memory for IP headers, flags, hints and other structs */
    uint8_t *packet = calloc(IP_MAXPACKET, sizeof(uint8_t));
    struct addrinfo *hints = calloc(1, sizeof(struct addrinfo));
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
        fprintf(stderr, "inet_ntop() failed.\nError message: %s", strerror(status));
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(resolved_target);

    /* --- IPv4 Header Stage --- */
    
    create_ipheader(&iphdr, info, 255, TCP);

    /* --- TCP Header Stage --- */

    if(packet_no == HEAD) {
        create_tcpheader(&iphdr, &tcphdr, info, info->head_port);
    }
    else if(packet_no == TAIL) {
        create_tcpheader(&iphdr, &tcphdr, info, info->tail_port);
    }
    else {
        fprintf(stderr, "Unsupported packet number (1 or 0 only). Exiting now!\n");
        free(packet);
        free(hints);
        free(dst_ip);
        exit(EXIT_FAILURE);
    }

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
    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket() failed ");
        free(packet);
        free(hints);
        free(dst_ip);
        exit(EXIT_FAILURE);
    }

    /* We need to tell the kernel that we'll be adding our own IP header */
    // Set flag so socket expects us to provide IPv4 header.
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt() failed to set IP_HDRINCL ");
        free(packet);
        free(hints);
        free(dst_ip);
        exit(EXIT_FAILURE);
    }

    // Bind socket to interface index.
    if(setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifr, sizeof(*ifr)) < 0) {
        perror("setsockopt() failed to bind to interface ");
        free(packet);
        free(hints);
        free(dst_ip);
        exit(EXIT_FAILURE);
    }

    /* Now we need to sniff for the RST packet while the TCP packet is being sent */

    pid_t child = fork();
    if(child == 0) {
        // Wait for RST flag
        get_rst(info);
        printf("Retrieved RST Packet :P\n");
        exit(EXIT_SUCCESS);
    } else if(child == -1) {
        /* Something went wrong */
        perror("fork ");
        free(packet);
        free(hints);
        free(dst_ip);
        exit(EXIT_FAILURE);
    } else {
        // Send packet.
        if(sendto(sockfd, packet, IP4_HDRLEN + TCP_HDRLEN, 0, (const struct sockaddr *) &sin, sizeof(struct sockaddr)) < 0)  {
            perror("sendto() failed ");
            free(packet);
            free(hints);
            free(dst_ip);
            exit(EXIT_FAILURE);
        }
        printf("Sent TCP Packet :)\n");
        wait(&child);
    }

    free(packet);
    free(hints);
    free(dst_ip);

    close(sockfd);

    return 0;
}