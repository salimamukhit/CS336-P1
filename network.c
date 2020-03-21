#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>        
#include <sys/ioctl.h>        
#include <bits/ioctls.h>      
#include <net/if.h>           
#include <linux/if_ether.h>   
#include <linux/if_packet.h>  
#include <net/ethernet.h>

#include <errno.h>

// Configuration file fills our structs

// UDP connection being established with the server

// 12000 packet trains are bing filled with high entropy and low entropy data

// Send packet trains

// We receive the ICMP packets that contain the arrival times

// We decide whether compression took place or no