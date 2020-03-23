/**
* @file Handles all the UDP train sending and making.
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

unsigned short csum(unsigned short *buf, int nwords) {


// Source IP, source port, target IP, target port from the command line arguments
int udp_train(int argc, char * argv[]);

#endif