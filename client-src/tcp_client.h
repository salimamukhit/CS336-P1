/**
* @file Handles all TCP communications for the client.
*/ 

#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "ini_parser.h"

int send_config(struct ini_info *info);
int receive_results(unsigned short int port, struct ini_info *info, double *low_arrival, double *high_arrival);

#endif