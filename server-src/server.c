#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "tcp_server.h"
#include "udp_server.h"
#include "logger.h"

#define RED_CODE "\033[31m"
#define GREEN_CODE "\033[32m"
#define BLUE_CODE "\033[34m"
#define RESET_CODE "\033[0m"
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
    if(argc > 2) {
        fprintf(stderr, "Too many arguments!\n");
        exit(EXIT_FAILURE);
    }
    if(argc == 1) {
        printf("%sNO PORT NUMBER SPECIFIED!\n", RED_CODE);
        printf("%sDefaulting to Port:%s 50000%s\n\n",GREEN_CODE, BLUE_CODE, RESET_CODE);
        goto start;  // This is technically more like assembler code, but it looks pretty. Look for the label "start:"
    }

    LOG("Command Line Port: %s\n", argv[1]);
    char *port_string = argv[1];
    for(int i = 0; i < 7; i++) {
        char curr = *(port_string + i);
        if(curr == '\0') break;
        if(isdigit(curr) == 0) {
            fprintf(stderr, "The port number is not a number! Please choose one in the range 49152 to 65535\n");
            exit(EXIT_FAILURE);
        }
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

    printf("Configured Port:  %d", port);


start:
    /* Listen for the config file and if there is error don't continue */ /* Specify step number 0 as well */
    if(start_server((u_int16_t) port, CONFIG_STEP) < 0) {
        perror("Something happend in the TCP connection for retriving the .INI");
        exit(EXIT_FAILURE);
    }
    printf("Hopefully retrieved the INI file :)\n");
    
    /* Listen for the UDP files and create the analytics. */
    /* This function should automatically call the next step to send back the results */
    // TODO create a UDP server...

    
    return 0;
}