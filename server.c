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

#define PORT 35000

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

int main(int argc, char *argv[]) {
    printf("HI\n");

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