#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>        
#include <sys/ioctl.h>        
#include <bits/ioctls.h>      
#include <net/if.h>           
#include <linux/if_ether.h>   
#include <linux/if_packet.h>  
#include <net/ethernet.h>

#include <errno.h>

#include "next_token.h" // Needed for parsing the config file

/* Setting up all the structs */
struct udpheader {
    unsigned short int udph_srcport;
    unsigned short int udph_destport;
    unsigned short int udph_len;
    unsigned short int udph_chksum;
};

struct udppacket {
    struct udpheader udp_header;
    uint8_t payload_size;
    uint8_t ttl;
    char data[];
};

/* Method that parses configuration file and passes all the parameters where necessary */
/* Returns 1 if parsing was successful, 0 otherwise*/
int parse_ini(struct sockaddr_in *server_address, struct udpheader *train_udp, struct sockaddr_in *client_address, struct udppacket *one_packet, time_t *meas_time, short int *packet_num) {
    FILE* fp = fopen("received_config.ini", "r");
    if (fp == NULL) {
        return 0;
    }
    char line_arr[256];
    while (fgets(line_arr, 256, fp) != NULL) {
        char *line = line_arr; // per line there should be two tokens
        char *value;
        if (returnValue(line, "ServerIP") != NULL) {
            value = returnValue(line, "ServerIP");
            inet_pton(AF_INET, value, &(server_address->sin_addr));
        } else {
            return 0;
        }
        if (returnValue(line, "SourceUDP") != NULL) {
            value = returnValue(line, "SourceUDP");
            train_udp->udph_srcport = atoi(value);
        } else {
            return 0;
        }
        if (returnValue(line, "DestinationUDP") != NULL) {
            value = returnValue(line, "DestinationUDP");
            train_udp->udph_destport = atoi(value);
        } else {
            return 0;
        }
        if (returnValue(line, "HeadDestinationTCP") != NULL) {
            value = returnValue(line, "HeadDestinationTCP");
            client_address->sin_port = atoi(value);
        } else {
            return 0;
        }
        if (returnValue(line, "TailDestinationTCP") != NULL) {
            value = returnValue(line, "TailDestinationTCP");
            client_address->sin_port = atoi(value);
        } else {
            return 0;
        }
        if (returnValue(line, "PortNumberTCP") != NULL) {
            value = returnValue(line, "PortNumberTCP");
            server_address->sin_port = atoi(value);
        } else {
            return 0;
        }
        if (returnValue(line, "PayloadSizeUDP") != NULL) {
            value = returnValue(line, "PayloadSizeUDP");
            one_packet->payload_size = atoi(value);
        } else {
            return 0;
        }
        if (returnValue(line, "InterMeasurementTime") != NULL) {
            value = returnValue(line, "InterMeasurementTime");
            meas_time = (time_t)atoi(value);
        } else {
            return 0;
        }
        if (returnValue(line, "NumberPackets") != NULL) {
            value = returnValue(line, "NumberPackets");
            packet_num = atoi(value);
        } else {
            return 0;
        }
        if (returnValue(line, "TimeToLiveUDP") != NULL) {
            value = returnValue(line, "TimeToLiveUDP");
            one_packet->ttl = atoi(value);
        } else {
            return 0;
        }
    }
    return 1;
}

char returnValue(char* line, char var_name[]) {
    char *name;
    char *value;
    name = next_token(&line, "=");
    value = next_token(&line, "=");
    if (strcmp(name, var_name) == 0) {
        return value;
    }
    return NULL;
}

/* Our main function that performs configuration for all clients and servers*/
int main() {
    /* Declaration of all structs that we will pass to respective clients and operate on*/
    struct sockaddr_in server_address; // Socket address of a server
    struct sockaddr_in client_address; // Socket address of a client
    struct udpheader train_udp; // UDP header for a packet in a packet train
    struct udppacket one_packet; // One packet from a train
    time_t meas_time;
    unsigned short int packet_num;
    if (parse_ini(&server_address, &train_udp, &client_address, &one_packet, &meas_time, &packet_num) == 0) {
        return EXIT_FAILURE;
    }
    one_packet.udp_header = train_udp;
    // establish UDP communication between a udp client and a server
    // establish TCP communication between a server and tcp client
    return EXIT_SUCCESS;
}