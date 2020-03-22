#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
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

#include "next_token.h"

#define PORT 35000
#define MAX 1024

// Setup socket()

// Specifying Socket Connection Information
// IMPORTANT: this can be cast to a (struct sockaddr) later
//struct sockaddr_in {
//    unsigned short sin_family; /* Address Family for the Socket IN (AF_INET) */
//    unsigned short sin_port; /* Address Port for the Socket IN (16 bits) */
//    struct in_addr sin_addr; /* Internet Address for the Socket IN (32 bits) */
//    char sin_zero[8];       /* Not used */
//}

// bind()

// listen()

// accept()

// recv()

// send() - loop to recv()

// close()

void parse_ini(struct sockaddr_in *servaddr, int socket_fd) {
    FILE* fp = fopen("received_config.ini", "r");
    if(fp == NULL) {
        printf("bad file...\n");
        return;
    }
    char line_arr[150];
    while(fgets(line_arr, 256, fp) != NULL) {
        char *line = line_arr;
        // per line there should be two tokens
        char *name;
        char *value;
        name = next_token(&line, "=");
        value = next_token(&line, "=");
        if (strcmp(name, "ServerIP") == 0) {
            inet_pton(AF_INET, value, &(servaddr->sin_addr));
            //servaddr->sin_addr = inet_aton(value);
        }
        // Have some logic for detecting names and assign them to proper variables or struct
        
    }
}

void retrieve_config(FILE *file, struct sockaddr_in *servaddr, int socket_fd) {
    char buff[MAX] = { 0 };  // to store message from client

    FILE *fp = fopen("received_config.ini","w"); // stores the file content in recieved.txt in the program directory

    if(fp == NULL){
        printf("Error IN Opening File ");
        return ;
    }

    while(read(socket_fd, buff, MAX) > 0)
        fprintf(fp,"%s",buff);

    printf("File received successfully !! \n");
    printf("New File created is received.txt !! \n");
}

int main(int argc, char *argv[]) {
    printf("HI\n");
    // parse_ini();

    // Setup socket()

    // bind()

    // listen()

    // accept()

    // recv()

    // send() - loop to recv()

    // close()


    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if (sockfd == -1) {
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
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli);
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (struct sockaddr*) &cli, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n");
    

    // Read client data:
    char read_buffer[1024] = { 0 };
    read(connfd, read_buffer, 1024);
    printf("Client says: %s\n", read_buffer);
  
    // Function for chatting between client and server
    char *response_buf = "Hi there bruh!";
    write(connfd, response_buf, strlen(response_buf));
  
    // After chatting close the socket 
    close(sockfd); 

    return 0;
}