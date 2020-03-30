#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "ini_parser.h"
#include "next_token.h"
#include "tcp_server.h"

#define MAX_CONFIG_SIZE 4096

/**
 * @brief Holds the information gathered from the INI.
 */
struct udp_pckt_params {
    unsigned int payload_size;
    unsigned int measurement_time;
    unsigned int number_packets;
};

/**
 * @brief Reads from the socket and writes the recieved INI to a file.
 * 
 * @param file 
 * @return int success for failure
 */
int retrieve_config(int socket_fd) {
    char buff[MAX_CONFIG_SIZE] = { 0 };  // to store message from client

    FILE *fp = fopen("received_config.ini","w"); // stores the file content in recieved.txt in the program directory

    if(fp == NULL){
        printf("Error IN Opening File ");
        return -1;
    }

    while(read(socket_fd, buff, MAX_CONFIG_SIZE) > 0)
        fprintf(fp,"%s",buff);

    printf("File received successfully !! \n");
    printf("New File created is received.txt !! \n");

    return 0;
}

/**
 * @brief Listens on specified port for a incomming connection and sends 
 * 
 * @param port 
 * @return int 
 */
int start_server(u_int16_t port, int step_num) {
    int sockfd, connfd;
    unsigned int len; 
    struct sockaddr_in servaddr, cli; 
  
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
        printf("Listen failed...\n"); 
        return -1;
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli);
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (struct sockaddr*) &cli, &len); 
    if(connfd < 0) { 
        printf("server acccept failed...\n"); 
        return -1;
    } 
    else
        printf("server acccept the client...\n");
    

    // Read client data:
    char read_buffer[8096] = { 0 };
    read(connfd, read_buffer, 8096);
    printf("Client says: %s\n", read_buffer);
  
    // Function for chatting between client and server
    char *response_buf = "Hi there bruh!";
    write(connfd, response_buf, strlen(response_buf));
  
    // After chatting close the socket 
    close(sockfd);
    return 0;
}