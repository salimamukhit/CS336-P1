/**
* @file Handles all the UDP train sending and making.
*/ 

#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

#include <sys/types.h>

#include "../shared-src/structs.h"

void fillTrain(char** train, unsigned short int num, unsigned int size, int type);

int udp_train(struct ini_info *info);

#endif
