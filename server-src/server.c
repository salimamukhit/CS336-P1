#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "tcp_server.h"
#include "udp_server.h"
#include "../shared-src/ini_parser.h"
#include "../shared-src/logger.h"
#include "../shared-src/structs.h"

#define RED_CODE "\033[31m"
#define GREEN_CODE "\033[32m"
#define YELLOW_CODE "\033[33m"
#define BLUE_CODE "\033[34m"
#define RESET_CODE "\033[0m"

#define EMPEMERAL_START 49152
#define PORT_MAX 65535
#define DEFAULT_PORT 50000

#define UDP_STEP 1
#define INI_NAME "received.ini"


/**
 * @brief Starts the server and handles the transitioning between the TCP -> UDP -> TCP steps
 * 
 * @param argc the number of arguments
 * @param argv the command line instruction containing the name of config file
 * @return int success or failure
 */
int main(int argc, char *argv[]) {
    /* Read in TCP port from server_config.ini */
    struct ini_info *tcp_port_ini = calloc(1, sizeof(struct ini_info));
    if(argc == 1) {
        strcpy(tcp_port_ini->file_name, "server_config.ini");
        printf("%sServer configuration file not provided, defaulting to:%s '%sserver_config.ini%s'\n",
                GREEN_CODE, RESET_CODE, BLUE_CODE, RESET_CODE);
    } else if(argc >= 2) {
        strcpy(tcp_port_ini->file_name, argv[1]);
    }
    
    parse_ini(tcp_port_ini);

    unsigned short port = tcp_port_ini->server_port; //DEFAULT_PORT;


    /* Prepare the INI struct */
    struct ini_info *info = calloc(1, sizeof(struct ini_info));
    strcpy(info->file_name, INI_NAME);

    if(argc > 2) {
        fprintf(stderr, "Too many arguments!\n");
        exit(EXIT_FAILURE);
    }

    if(port < 1) {
        fprintf(stderr, "C'mon you can't have a negative port! Please choose one in the range 49152 to 65535\n");
    }
    else if(port < EMPEMERAL_START) {
        fprintf(stderr, "Not an empemeral port! Please choose one in the range 49152 to 65535\n");
    }
    else if(65535 < PORT_MAX) {
        fprintf(stderr, "Port is not in the valid range! Please choose on in the range 49152 to 65535\n");
    }

    printf("%sConfigured Port:%s %d\n%s",RED_CODE, YELLOW_CODE, port, RESET_CODE);

    /* Listen for the config file and if there is error don't continue */ /* Specify step number 0 as well */
    if(start_server((u_int16_t) port, info) < 0) {
        perror("Something happend in the TCP connection for retriving the .INI");
        exit(EXIT_FAILURE);
    }
    if(parse_ini(info) != 0) {
        perror("Couldn't parse the ini file!\n");
    } else {
        printf("INI info successfully retrieved!\n");
    }

    // printf("Hopefully retrieved the INI file :)\n"); // we use this for debugging

    double low_arrival, high_arrival;
    start_udp_server(info, &low_arrival, &high_arrival);
    send_results(port, &low_arrival, &high_arrival);
    
    free(info);
    /* Remove temp INI file */
    remove(INI_NAME);
    return 0;
}