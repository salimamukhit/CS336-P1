/**
* @file Parses a INI file.
*/
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

/**
* @file Parses a INI file
*/ 

#ifndef _INI_PARSER_H_
#define _INI_PARSER_H_

#include <sys/types.h>
#include "../client-src/udp_client.h"

/* struct with info from ini to be returned */
struct ini_info {
    char file_name[1024];
    struct in_addr server_ip;
    struct udpheader train_udp;
    unsigned short head_port;
    unsigned short tail_port;
    unsigned short server_port;
    unsigned short int payload_size;
    time_t meas_time;
    unsigned short int packet_num;
    unsigned short int packet_ttl;
};

/**
 * @brief Returns the NAME=VALUE pair from the provided line.
 * 
 * @param line the provided line
 * @param name a zeroed out array in which to put the name
 * @param value a zeroed out array in which to put the value
 * @return char 
 */
int extractor(char* line, char name[], char value[]);

/**
 * @brief Parses configuration file and passes all the parameters where necessary.
 * @param parsed_info is a pointer to the information to be distributed
 * @return 0 for success, -1 for failure
 */
int parse_ini(struct ini_info *parsed_info);

/**
 * @brief Returns the NAME=VALUE pair from the provided line.
 * 
 * @param line the provided line
 * @param var_name name that we are looking for
 * @return char 
 */
char returnValue(char* line, char var_name[]);

#endif
