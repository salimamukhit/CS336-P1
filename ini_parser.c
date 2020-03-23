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
#include "udp_client.h"
#include "udp_server.h"

/* struct with infro from ini to be returned */

struct ini_info {
    char *file_name;
    struct in_addr server_ip;
    struct udpheader train_udp;
    unsigned short head_port;
    unsigned short tail_port;
    unsigned short client_port;
    uint8_t payload_size;
    time_t meas_time;
    uint8_t packet_num;
    uint8_t packet_ttl;
};

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
 * @param parsed_info is a pointer to the information to be distributed
 * @return int returns ini_info type information if parsing was successful, NULL otherwise
 */
struct ini_struct* parse_ini(struct ini_info *parsed_info) {
    FILE* fp = fopen(parsed_info->file_name, "r");
    if(fp == NULL) {
        return NULL;
    }
    char line_arr[256];
    while(fgets(line_arr, 256, fp) != NULL) {
        char *line = line_arr; // per line there should be two tokens
        char *value;
        
        if(returnValue(line, "ServerIP") != NULL) {
            value = returnValue(line, "ServerIP");
            inet_pton(AF_INET, value, &(parsed_info->server_ip));
        } else if(returnValue(line, "SourceUDP") != NULL) {
            value = returnValue(line, "SourceUDP");
            parsed_info->train_udp.udph_srcport = (unsigned short int)atoi(value);
        } else if(returnValue(line, "DestinationUDP") != NULL) {
            value = returnValue(line, "DestinationUDP");
            parsed_info->train_udp.udph_destport = (unsigned short int)atoi(value);
        } else if(returnValue(line, "HeadDestinationTCP") != NULL) {
            value = returnValue(line, "HeadDestinationTCP");
            parsed_info->head_port = (unsigned short int)atoi(value);
        } else if(returnValue(line, "TailDestinationTCP") != NULL) {
            value = returnValue(line, "TailDestinationTCP");
            parsed_info->tail_port = (unsigned short int)atoi(value);
        } else if(returnValue(line, "PortNumberTCP") != NULL) {
            value = returnValue(line, "PortNumberTCP");
            parsed_info->client_port = (unsigned short int)atoi(value);
        } else if(returnValue(line, "PayloadSizeUDP") != NULL) {
            value = returnValue(line, "PayloadSizeUDP");
            parsed_info->payload_size = (uint8_t)atoi(value);
        } else if(returnValue(line, "InterMeasurementTime") != NULL) {
            value = returnValue(line, "InterMeasurementTime");
            parsed_info->meas_time = (time_t)atoi(value);
        } else if(returnValue(line, "NumberPackets") != NULL) {
            value = returnValue(line, "NumberPackets");
            parsed_info->packet_num = (uint8_t)atoi(value);
        } else if(returnValue(line, "TimeToLiveUDP") != NULL) {
            value = returnValue(line, "TimeToLiveUDP");
            parsed_info->packet_ttl = (uint8_t)atoi(value);
        } else {
            return NULL;
        }
    }
    return parsed_info;
}