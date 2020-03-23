#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

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

#include "ini_parser.h"
#include "next_token.h"

/**
 * @brief Returns the NAME=VALUE pair from the provided line.
 * 
 * @param line the provided line
 * @param var_name name that we are looking for
 * @return char 
 */
char returnValue(char* line, char var_name[]) {
    char *name;
    char *value;
    name = next_token(&line, "=");
    value = next_token(&line, "=");
    if(strcmp(name, var_name) == 0) {
        return value;
    }
    return NULL;
}

/**
 * @brief Parses configuration file and passes all the parameters where necessary.
 * 
 * @param file_name the name of the configuration file
 * @param server_address the struct with the socket configuration that we need to build.
 * @param train_udp a UDP header that the train of information will use.
 * @param client_address 
 * @param one_packet 
 * @param meas_time the Inter-Measurement time
 * @param packet_num 
 * @return int Returns 1 if parsing was successful, 0 otherwise
 */
int parse_ini(char *file_name, struct sockaddr_in *server_address, struct udpheader *train_udp,
        struct sockaddr_in *client_address, struct udppacket *one_packet, time_t *meas_time, short int *packet_num) {
    FILE* fp = fopen(file_name, "r");
    if(fp == NULL) {
        return 0;
    }
    char line_arr[256];
    while(fgets(line_arr, 256, fp) != NULL) {
        char *line = line_arr; // per line there should be two tokens
        char *value;
        
        if(returnValue(line, "ServerIP") != NULL) {
            value = returnValue(line, "ServerIP");
            inet_pton(AF_INET, value, &(server_address->sin_addr));
        } else if(returnValue(line, "SourceUDP") != NULL) {
            value = returnValue(line, "SourceUDP");
            train_udp->udph_srcport = atoi(value);
        } else if(returnValue(line, "DestinationUDP") != NULL) {
            value = returnValue(line, "DestinationUDP");
            train_udp->udph_destport = atoi(value);
        } else if(returnValue(line, "HeadDestinationTCP") != NULL) {
            value = returnValue(line, "HeadDestinationTCP");
            client_address->sin_port = atoi(value);
        } else if(returnValue(line, "TailDestinationTCP") != NULL) {
            value = returnValue(line, "TailDestinationTCP");
            client_address->sin_port = atoi(value);
        } else if(returnValue(line, "PortNumberTCP") != NULL) {
            value = returnValue(line, "PortNumberTCP");
            server_address->sin_port = atoi(value);
        } else if(returnValue(line, "PayloadSizeUDP") != NULL) {
            value = returnValue(line, "PayloadSizeUDP");
            one_packet->payload_size = atoi(value);
        } else if(returnValue(line, "InterMeasurementTime") != NULL) {
            value = returnValue(line, "InterMeasurementTime");
            meas_time = (time_t)atoi(value);
        } else if(returnValue(line, "NumberPackets") != NULL) {
            value = returnValue(line, "NumberPackets");
            packet_num = atoi(value);
        } else if(returnValue(line, "TimeToLiveUDP") != NULL) {
            value = returnValue(line, "TimeToLiveUDP");
            one_packet->ttl = atoi(value);
        } else {
            return 0;
        }
    }
    return 1;
}