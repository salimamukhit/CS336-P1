#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "tcp_server.h"
#include "udp_server.h"

#define EMPEMERAL_START 49152
#define PORT_MAX 65535
#define CONFIG_STEP 0
#define UDP_STEP 1

/**
 * @brief Starts the server and handles the transitioning between the TCP -> UDP -> TCP steps
 * 
 * @param argc the number to arguments
 * @param argv the command line instruction containing the port number (e.g. {"server", "1337"})
 * @return int success for failure
 */
int main(int argc, char *argv[]) {
    if(argc != 1) {
        fprintf(stderr, "No port number specified or too many arguments!\n");
        exit(EXIT_FAILURE);
    }
    if(isdigit(argv[1]) == 0) {
        fprintf(stderr, "The port number is not a number! Please choose one in the range 49152 to 65535\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    if(port < 1) {
        fprintf(stderr, "C'mon you can't have a negative port! Please choose one in the range 49152 to 65535\n");
    }
    else if(port < EMPEMERAL_START) {
        fprintf(stderr, "Not an empemeral port! Please choose one in the range 49152 to 65535\n");
    }
    else if(65535 < PORT_MAX) {
        fprintf(stderr, "Port is not in the valid range! Please choose on in the range 49152 to 65535\n");
    }

    /* Listen for the config file and if there is error don't continue */ /* Specify step number 0 as well */
    if(start_server((u_int16_t) port, CONFIG_STEP) < 0) {
        perror("Something happend in the TCP connection for retriving the .INI");
        exit(EXIT_FAILURE);
    }
    
    /* Listen for the UDP files and create the analytics. */
    /* This function should automatically call the next step to send back the results */
    // TODO create a UDP server...

    
    return 0;
}