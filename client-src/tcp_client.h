/**
* @file Handles all TCP communications for the client.
*/ 

#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "ini_parser.h"

int send_config(int argc, char *argv[], struct ini_info *info);

#endif