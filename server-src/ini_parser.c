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
#include "logger.h"

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

/**
 * @brief Parses configuration file and passes all the parameters where necessary.
 * @param parsed_info is a pointer to the information to be distributed
 * @return int returns ini_info type information if parsing was successful, NULL otherwise
 */
struct ini_info* parse_ini(struct ini_info *parsed_info) {
    LOG("file_name: \"%s\"\n", parsed_info->file_name);
    FILE* fp = fopen(parsed_info->file_name, "r");
    LOG("FILE*: %p\n", fp);
    if(fp == NULL) {
        return NULL;
    }
    
    char line_arr[256] = { 0 };
    while(fgets(line_arr, 256, fp) != NULL) {
        // Strip the newline character from the end
        *(line_arr + strnlen(line_arr, 256)-1) = '\0';

        char *line = line_arr; // per line there should be two tokens
        char value[128] = { 0 };
        char name[128] = { 0 };
        
        if(extractor(line, name, value) == -1) return NULL;
        
        // BUG is that line gets used up by next token
        if(strncmp(name, "ServerIP", 255) == 0) {
            LOGP("Server IP!!!!\n");
            inet_pton(AF_INET, value, &(parsed_info->server_ip));
        } 
        else if(strncmp(name, "SourceUDP", 255) == 0) {
            parsed_info->train_udp.udph_srcport = (unsigned short int)atoi(value);
        } 
        else if(strncmp(name, "DestinationUDP", 255) == 0) {
            parsed_info->train_udp.udph_destport = (unsigned short int)atoi(value);
        } 
        else if(strncmp(name, "HeadDestinationTCP", 255) == 0) {
            parsed_info->head_port = (unsigned short int)atoi(value);
        } 
        else if(strncmp(name, "TailDestinationTCP", 255) == 0) {
            parsed_info->tail_port = (unsigned short int)atoi(value);
        } 
        else if(strncmp(name, "PortNumberTCP", 255) == 0) {
            parsed_info->client_port = (unsigned short int)atoi(value);
        } 
        else if(strncmp(name, "PayloadSizeUDP", 255) == 0) {
            parsed_info->payload_size = (unsigned short int)atoi(value);
        } 
        else if(strncmp(name, "InterMeasurementTime", 255) == 0) {
            parsed_info->meas_time = (time_t)atoi(value);
        } 
        else if(strncmp(name, "NumberPackets", 255) == 0) {
            parsed_info->packet_num = (unsigned short int)atoi(value);
        } 
        else if(strncmp(name, "TimeToLiveUDP", 255) == 0) {
            parsed_info->packet_ttl = (unsigned short int)atoi(value);
        } 
        else {
            return NULL;
        }
    }
    return parsed_info;
}
