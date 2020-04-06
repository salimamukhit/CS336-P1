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
#include <netinet/ip.h>
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
#define TARGET "www.google.com"
#define MY_IP "127.0.0.1"

/* 
Standalone application
Step 1: Retrieves MAC address of a local machine in order to let packets go through it
Step 2: Identifies a receiver machine, fills the fields with pseudo data. Chosen IP: google
Step 3: Creates TCP SYN packets for head and tail
Step 4: Creates UDP packets for the rest of the packet trains
Step 5: Sends this data on different ports and starts timer
Step 6: Receives RST packets
*/
int main() {
    struct ini_info* info;
    char* interface, ip_src, target;
    struct ifreq ifr;
    uint8_t mac_src[6], mac_dest[6];
    strcpy(info->file_name, INI_NAME);

    // parsing the ini file
    if (parse_ini(info) != 0) {
        printf("Problem with parsing the INI file.\n");
        return EXIT_FAILURE;
    }
    printf("Successfully parsed the ini file.\n");
    
    printf("Starting the standalone compression detection application.\n");

    // setting up the mac address to let packet go through it
    interface = calloc(1, sizeof(INTERFACE));
    strcpy(interface, INTERFACE);
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    // getting a socket for retrieving mac address
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

    // destination mac address doesn't matter
    for(int i = 0; i < 6; i++) {
        mac_dest[i] = 0xff;
    }

    // create tcp packets for head and tail for both trains

    // create udp packets for high and low entropy trains
    char** low_train; // low entropy train
    char** high_train; // high entropy train

    fillTrain(low_train, info->packet_num, info-> payload_size, 0);
    fillTrain(high_train, info->packet_num, info-> payload_size, 1);

    int udp_packets = info->packet_num - 2; // number of packets sent with UDP, exclude head and tail
    struct udphdr udp;
    struct iphdr ip;

    memset(&ip, 0, sizeof(ip));
    memset(&udp, 0, sizeof(udp));

    strcpy(ip_src, MY_IP);
    strcpy(target, TARGET);

    // send head and start timer
    // send packet train
    for(int i = 0; i < udp_packets; i++) {
        // send
    }
    // send tail and start timer
    int sock_r;
    sock_r = socket(AF_PACKET, SOCK_RAW, ETH_P_ALL);
    if(sock_r < 0) {
        perror("Raw socket");
        return EXIT_FAILURE;
    }
    print("Receiver socket was created");

    unsigned char *buffer = (unsigned char *) malloc(65536); // to receive data
    memset(buffer, 0, 65536);
    struct sockaddr saddr;
    int saddr_len = sizeof(saddr);
    int rst_count = 0;
    struct sockaddr_in src, dest;

    while(1) {
        if(rst_count == 2) {
            break;
        }
        int buflen = recvfrom(sock_r, buffer, 65536, 0, &saddr, (socklen_t*)&saddr_len);
        if(buflen < 0) {
            printf("error in reading recvfrom function\n");
            return -1;
        }
        struct ethhdr *eth = (struct ethhdr*)(buffer);
        if(eth->h_proto == 8) {
            printf("Received an IP PROTO packet\n");
            unsigned short iphdrlen;
            struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
            memset(&src, 0, sizeof(src));
            src.sin_addr.s_addr = ip->saddr;
            memset(&dest, 0, sizeof(dest));
            dest.sin_addr.s_addr = ip->daddr;
            if(ip->protocol == 6) {
                printf("There is a TCP packet");
                iphdrlen = ip->ihl * 4;
                struct tcphdr *tcp = (struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
                if(tcp->rst == 1) {
                    if(tcp->source == info->tail_port) {
                        rst_count++;
                    } else if(tcp->source == info->head_port) {
                        rst_count++;
                    }
                }
            }
        }
    }
    // receive info for both head and tail and stop timers
    // repeat again for high entropy train
    return EXIT_SUCCESS;