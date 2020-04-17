#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "../shared-src/ini_parser.h"
#include "../shared-src/next_token.h"
#include "tcp_server.h"
#include "../shared-src/logger.h"
#include "../shared-src/structs.h"

#define MAX_CONFIG_SIZE 4096

static int sockfd;

/**
 * @brief Reads from the socket and writes the recieved INI to a file.
 * 
 * @param file 
 * @return int success for failure
 */
int retrieve_config(int socket_fd) {
    char buff[MAX_CONFIG_SIZE] = { 0 };  // to store message from client

    FILE *fp = fopen("received_config.ini","w"); // stores the file content in recieved.txt in the program directory

    if(fp == NULL) {
        printf("Error IN Opening File ");
        return -1;
    }

    while(read(socket_fd, buff, MAX_CONFIG_SIZE) > 0)
        fprintf(fp,"%s",buff);

    printf("File received successfully !! \n");
    printf("New File created is received.txt !! \n");

    return 0;
}

int write_file(char buf[], char *file_name) {
    FILE *ini;
    ini = fopen(file_name, "w");

    if(ini == NULL) {
        fprintf(stderr, "Can't open output file %s!\n", file_name);
        return -1;
    }

    int fd = fileno(ini);
    write(fd, buf, strnlen(buf, 8095));
    close(fd);
    return 0;
}

/**
 * @brief Listens on specified port for a incomming connection and sends a 
 *        confimation back to the client.
 * 
 * @param port 
 * @return int 
 */
int start_server(u_int16_t port, struct ini_info *info) {
    int connfd;
    unsigned int len; 
    struct sockaddr_in servaddr, cli; 
    printf("Note: The INI can be at max 255 lines long!\n");

    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if(sockfd == -1) {
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else {
        printf("Socket successfully created..\n");
    }
    bzero(&servaddr, sizeof(servaddr));

    // Set socket options, make sure kernel doesn't keep it open
    int true = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port); 
  
    // Binding newly created socket to given IP and verification 
    if((bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        return -1;
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if((listen(sockfd, 5)) != 0) { 
        perror("Listen failed...\n"); 
        return -1;
    } 
    else
        printf("Server listening..\n"); 

    len = sizeof(cli);
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (struct sockaddr*) &cli, &len); 
    if(connfd < 0) { 
        printf("server accept failed...\n"); 
        return -1;
    } 
    else
        printf("server accept the client...\n");
    

    // Read client data:
    char read_buf[8096] = { 0 };
    while(1) {
        char temp_buf[256] = { 0 };
        read(connfd, temp_buf, 256);
        if(strcmp("EOF", temp_buf) == 0) break;

        strncat(read_buf, temp_buf, 255);
    }

    char response_buf[256] = { 0 };

    // Write out ini to file
    if(write_file(read_buf, info->file_name) == -1) {
        strncpy(response_buf, "Failed to write file...", 255);
        write(connfd, response_buf, strlen(response_buf));
        close(sockfd);
        return -1;
    }
    write_file(read_buf, info->file_name);

    // Parse the INI
    if(parse_ini(info) == -1) {
        strncpy(response_buf, "Failed to parse the INI...", 255);
        write(connfd, response_buf, strlen(response_buf));
        close(sockfd);
        return -1;
    }
  
    // Function for chatting between client and server
    strcpy(response_buf, "Received the INI successfully");
    write(connfd, response_buf, strlen(response_buf));
    shutdown(connfd, SHUT_WR);
  
    return 0;
}

/**
 * @brief 
 * 
 * @param port on which server sends the results
 * @param low_arrival pointer to the low entropy train arrival time
 * @param high_arrival pointer to the high entropy train arrival time
 * @return 0 for success, -1 for failure
 */
int send_results(unsigned short int port, double* low_arrival, double *high_arrival) {
    sleep(2);
    int connfd;
    unsigned int len; 
    struct sockaddr_in cli;

    printf("Now the TCP server is going to send findings to the client.\n");

    // listening
    if((listen(sockfd, 5)) != 0) { 
        perror("Listen"); 
        return -1;
    } 
    else {
        printf("Server listening...\n"); 
    }

    len = sizeof(cli);
  
    // accepting the data from the client
    connfd = accept(sockfd, (struct sockaddr*) &cli, &len);
    if(connfd < 0) {
        perror("Server accept");
        return -1;
    } 
    else {
        printf("Server accepted the client\n");
    }

    // sending the results to the client
    char response[128];
    double difference = *high_arrival - *low_arrival;
    if(difference < 100) {
        sprintf(response, "low entropy time: %lf sec\n high entropy time: %lf sec\n no compression was detected\n", *low_arrival, *high_arrival);
    } else {
        sprintf(response, "low entropy time: %lf sec\n high entropy time: %lf sec\n compression was detected\n", *low_arrival, *high_arrival);
    }
    
    write(connfd, response, sizeof(response));
    printf("Sent the results to the client\n");

    close(sockfd);
    return 0;
}