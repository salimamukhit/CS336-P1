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

#include "udp_client.h"
#include "udp_server.h"

/**
* @file Parses a INI file
*/ 

#ifndef _INI_PARSER_H_
#define _INI_PARSER_H_

#include <sys/types.h>

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
 * @brief Parses configuration file and passes all the parameters where necessary.
 * @param parsed_info is a pointer to the information to be distributed
 * @return int returns ini_info type information if parsing was successful, NULL otherwise
 */
struct ini_struct* parse_ini(struct ini_info *parsed_info);

#endif