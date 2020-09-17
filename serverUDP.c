// server program for udp connection 
#include <stdio.h> 
#include <strings.h> 
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 

#define KXVER 3
#include "k.h"

// Safe UDP Payload
#define BUFFERLENGTH 500

// gcc -shared -fPIC -o serverUDP.so serverUDP.c

// Modified from https://www.geeksforgeeks.org/udp-server-client-implementation-c/?ref=rp 
K callback(I listenfd) {
    char buffer[BUFFERLENGTH]; 
    struct sockaddr_in cliaddr;
    bzero(&buffer, sizeof(buffer));
    int len = sizeof(cliaddr); 
    // Receive message from server
    int n = recvfrom(listenfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);  
    buffer[n] = '\0'; 
    R k(0, buffer, (K)0);
};

// Server code 
K recvUDP(K port) {    
    
    int listenfd; 
    struct sockaddr_in servaddr; 
    bzero(&servaddr, sizeof(servaddr)); 

    P(port->t != -KI, krr((S) "port int type"));

    // Create a UDP Socket 
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);         
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port->i); 
    servaddr.sin_family = AF_INET;  
   
    // bind server address to socket descriptor 
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 

    R sd1(listenfd, callback);
 }; 


