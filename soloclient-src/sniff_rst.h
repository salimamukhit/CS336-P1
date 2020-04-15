/**
* @file Handles the creation of the IP header.
*/

#ifndef _SNIFF_RST_H_
#define _SNIFF_RST_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <pcap.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include "../shared-src/structs.h"
#include "create_hdrs.h"
#include "logger.h"

/**
 * @brief Uses libpcap to retrive the RST packet.
 * 
 * @param info the struct from the INI.
 * @return int 0 for success and -1 for failure.
 */
int get_rst(struct ini_info *info);

#endif
