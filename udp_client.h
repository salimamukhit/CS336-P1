/**
* @file Retrieves the next token from a string.
*/ 

#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

#include <sys/types.h>

// Can create separate header file (.h) for all headers' structure
// The IP header's structure
struct ipheader {
    unsigned char iph_ihl: 5, iph_ver: 4;
    unsigned char iph_tos;
    unsigned short int iph_len;
    unsigned short int iph_ident;
    unsigned char iph_flag;
    unsigned short int iph_offset;
    unsigned char iph_ttl;
    unsigned char iph_protocol;
    unsigned short int iph_chksum;
    unsigned int iph_sourceip;
    unsigned int iph_destip;
};

// UDP header's structure
struct udpheader {
    unsigned short int udph_srcport;
    unsigned short int udph_destport;
    unsigned short int udph_len;
    unsigned short int udph_chksum;
};

void parse_ini(struct sockaddr_in *servaddr, int socket_fd, struct udpheader *client_udp);

void retrieve_config(FILE *file, struct sockaddr_in *servaddr, int socket_fd);

unsigned short csum(unsigned short *buf, int nwords) {

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
 *
 * @returns: char pointer to the next token in the string.
 */
int udp_train(int argc, char * argv[]);

#endif