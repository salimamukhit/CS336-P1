#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

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
#include "logger.h"
#include "structs.h"

/**
 * @brief Returns the NAME=VALUE pair from the provided line.
 * 
 * @param line the provided line
 * @param name a zeroed out array in which to put the name
 * @param value a zeroed out array in which to put the value
 * @return char 
 */
int extractor(char* line, char name[], char value[]) {
    if(line == NULL) return -1;
    LOG("TEMP LINE:  \"%s\"\n", line);

    char* token1 = next_token(&line, "=");
    strcpy(name, token1);
    LOG("Name: \"%s\"\n", name);

    char* token2 = next_token(&line, "=");
    strcpy(value, token2);
    LOG("Value: \"%s\"\n", value);

    if(name == NULL || value==NULL) return -1;
    else return 0;
}

void check_port(char *port) {
    for(int i = 0; i < 7; i++) {
        char curr = *(port + i);
        if(curr == '\0') break;
        if(isdigit(curr) == 0) {
            fprintf(stderr, "The port number is not a number! Please choose one in the range 49152 to 65535\n");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Parses configuration file and passes all the parameters where necessary.
 * @param parsed_info is a pointer to the information to be distributed
 * @return int returns ini_info type information if parsing was successful, NULL otherwise
 */
int parse_ini(struct ini_info *parsed_info) {
    LOG("file_name: \"%s\"\n", parsed_info->file_name);
    FILE* fp = fopen(parsed_info->file_name, "r");
    LOG("FILE*: %p\n", fp);
    if(fp == NULL) {
        return -1;
    }

    /* Set Defaults */
    parsed_info->payload_size = 1000;
    parsed_info->meas_time = 15;
    parsed_info->packet_num = 600;
    
    char line_arr[256] = { 0 };
    while(fgets(line_arr, 256, fp) != NULL) {
        // Strip the newline character from the end
        *(line_arr + strnlen(line_arr, 256)-1) = '\0';

        char *line = line_arr; // per line there should be two tokens
        char value[128] = { 0 };
        char name[128] = { 0 };

        if(extractor(line, name, value) == -1) return -1;
        
        // BUG is that line gets used up by next token
        if(strncmp(name, "ServerIP", 255) == 0) {
            int status = inet_pton(AF_INET, value, &(parsed_info->server_ip));
            if(status < 1) fprintf(stderr, "'ServerIP=%s' is not valid!", value), exit(EXIT_FAILURE);
        }
        else if(strncmp(name, "SourceUDP", 255) == 0) {
            check_port(value);
            parsed_info->train_udp.udph_srcport = (unsigned short int)atoi(value);
        }
        else if(strncmp(name, "DestinationUDP", 255) == 0) {
            check_port(value);
            parsed_info->train_udp.udph_destport = (unsigned short int)atoi(value);
        }
        else if(strncmp(name, "HeadDestinationTCP", 255) == 0) {
            check_port(value);
            parsed_info->head_port = (unsigned short int)atoi(value);
        }
        else if(strncmp(name, "TailDestinationTCP", 255) == 0) {
            check_port(value);
            parsed_info->tail_port = (unsigned short int)atoi(value);
        }
        else if(strncmp(name, "PortNumberTCP", 255) == 0) {
            check_port(value);
            parsed_info->server_port = (unsigned short int)atoi(value);
        }
        else if(strncmp(name, "PayloadSizeUDP", 255) == 0) {
            parsed_info->payload_size = (unsigned short int)atoi(value);
            if(parsed_info->payload_size == 0) {
                fprintf(stderr, "'PayloadSizeUDP=%d' Cannot be zero", parsed_info->payload_size);
                exit(EXIT_FAILURE);
            }
        }
        else if(strncmp(name, "InterMeasurementTime", 255) == 0) {
            parsed_info->meas_time = (time_t)atoi(value);
            if(parsed_info->meas_time == 0) {
                fprintf(stderr, "'InterMeasurementTime=%ld' Cannot be zero", parsed_info->meas_time);
                exit(EXIT_FAILURE);
            }
        }
        else if(strncmp(name, "NumberPackets", 255) == 0) {
            parsed_info->packet_num = (unsigned short int)atoi(value);
            if(parsed_info->packet_num == 0) {
                fprintf(stderr, "'NumberPackets=%ud' Cannot be zero", parsed_info->packet_num);
                exit(EXIT_FAILURE);
            }
        }
        else if(strncmp(name, "TimeToLiveUDP", 255) == 0) {
            parsed_info->packet_ttl = (unsigned short int)atoi(value);
            if(parsed_info->packet_ttl > 255) {
                fprintf(stderr, "'TimeToLiveUDP=%s' Invalid TTL!", value);
                exit(EXIT_FAILURE);
            }
        }
        else if(strncmp(name, "ClientIP", 255) == 0) {
            strncpy(parsed_info->client_ip, value, 16);
        }
        else if(strncmp(name, "Interface", 255) == 0) {
            strncpy(parsed_info->interface, value, 19);
        }
        else if(strncmp(name, "StandAloneDst", 255) == 0) {
            strncpy(parsed_info->standalone_dst, value, 16);
        }
        else {
            return -1;
        }
    }
    return 0;
}
