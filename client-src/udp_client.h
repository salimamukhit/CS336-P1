/**
* @file Handles all the UDP train sending and making.
*/ 

#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

#include <sys/types.h>

#include "../shared-src/structs.h"

// Can create separate header file (.h) for all headers' structure
// The IP header's structure

unsigned short csum(unsigned short *buf, int nwords);


// Source IP, source port, target IP, target port from the command line arguments
int udp_train(struct ini_info *info);

#endif
