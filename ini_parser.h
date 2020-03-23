/**
* @file Parses a INI file.
*/ 

#ifndef _INI_PARSER_H_
#define _INI_PARSER_H_

#include <sys/types.h>

/**
 * @brief Returns the NAME=VALUE pair from the provided line.
 * 
 * @param line the provided line
 * @param var_name name that we are looking for
 * @return char 
 */
char returnValue(char* line, char var_name[]);

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
        struct sockaddr_in *client_address, struct udppacket *one_packet, time_t *meas_time, short int *packet_num);

#endif