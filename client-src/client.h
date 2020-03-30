/**
* @file Handles all the UDP train sending and making.
*/ 

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <sys/types.h>

int parse_ini(struct sockaddr_in *server_address, struct udpheader *train_udp, struct sockaddr_in *client_address,
                struct udppacket *one_packet, time_t *meas_time, short int *packet_num);

char returnValue(char* line, char var_name[]);

/**
 * @brief Our main function that performs configuration for all clients and servers.
 *        It's main purpose is to parse the ini file
 * 
 * @return int the return code for failure or success.
 */
int start_client();

#endif