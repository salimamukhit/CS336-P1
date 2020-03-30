#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../shared-src/ini_parser.h"
#include "../shared-src/structs.h"
#include "../shared-src/logger.h"

#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

int start_udp_server(struct ini_info *info);


#endif