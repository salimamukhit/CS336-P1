#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

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

#include "ini_parser.h"
#include "udp_client.h"
#include "tcp_client.h"
#include "logger.h"

#define INI_NAME "config.ini"

/**
 * @brief Performs configuration for the client
 * 
 * @return int failure or success
 */
int main(int argc, char* argv[]) {
    LOGP("Client Started :)\n");
    /* Declaration of all structs that we will pass to respective clients and operate on*/
    /*struct sockaddr_in server_address; // Socket address of a server
    struct sockaddr_in client_address; // Socket address of a client
    struct udpheader train_udp; // UDP header for a packet in a packet train
    struct udppacket one_packet; // One packet from a train
    time_t meas_time;
    unsigned short int packet_num; */

    char *filename = "config.ini";
    LOG("Size of ini_info struct: %ld\n", sizeof(struct ini_info));
    struct ini_info *info = calloc(1, sizeof(struct ini_info));
    LOGP("Set struct ini_info and filename\n");
    strncpy(info->file_name, filename, 1023);
    LOGP("Copied file name into struct\n");

    if(parse_ini(info) == -1) {
        fprintf(stderr, "Failed to parse the INI file!\n");
        return EXIT_FAILURE;
    }
    printf("Successfully parsed INI file\n");

    send_config(info);

    /* Filling our data with info from retrieved ini struct */
    /* Information needed for a client: all of the fields */

    // FOR THE FOLLOWING WE WILL USE THE SELECTION INTEGER TO DECIDE WHICH SERVICE TO START
    // establish UDP communication between a udp client and a server
    // establish TCP communication between a server and tcp client


    return EXIT_SUCCESS;
}
