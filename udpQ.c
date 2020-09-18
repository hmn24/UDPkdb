// server program for udp connection 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <strings.h> 
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>

#define KXVER 3
#include "k.h"

// Safe UDP Payload
#define BUFFERLENGTH 500

// Pkrr Check
#define Pkrr(x,y) P(x,krr(y)) 

#define TIMEOUT_SEC 2

// gcc -shared -fPIC -o udpQ.so udpQ.c

Z __inline S c2s(S s,J n){S r=(S)malloc(n+1);R r?memcpy(r,s,n),r[n]=0,r:(S)krr((S)"wsfull");}

Z S kdb2str(K kdbStr) {R kdbStr->t == -KS ? kdbStr->s : c2s((S) kC(kdbStr), kdbStr->n);}

// Modified from 
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/?ref=rp 
// https://stackoverflow.com/questions/16163260/setting-timeout-for-recv-fcn-of-a-udp-socket
K callback(I listenfd) {
    char buffer[BUFFERLENGTH]; 
    struct sockaddr_in cliaddr;
    
    bzero(&buffer, sizeof(buffer));
    bzero(&cliaddr, sizeof(cliaddr)); 
    
    I len = sizeof(cliaddr); 
    
    // Receive message from server
    I n = recvfrom(listenfd, buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);  
    buffer[n] = '\0'; 
    
    // Send acknowledgement back to client
    sendto(listenfd, " ", 1, MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len); 

    R k(0, buffer, (K)0);
 };

// Check sockets for timeout
I checkSocketTimeout(I socketfd) {
    
    // Set timer for recv_socket
    struct timeval timeout={TIMEOUT_SEC,0};
    setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (S)&timeout, sizeof(struct timeval));

    char buffer[1];
    I len;

    R recvfrom(socketfd, (S)buffer, 1, MSG_WAITALL, NULL, &len);
 };

// Server code 
K recvUDP(K port) {    
    
    I listenfd; 
    struct sockaddr_in servaddr; 
    bzero(&servaddr, sizeof(servaddr)); 

    Pkrr(port->t != -KI, "port int type");

    // Create a UDP Socket 
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);         
    Pkrr(0 > listenfd, "socket creation failed");
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port->i); 
    servaddr.sin_family = AF_INET;  
   
    // bind server address to socket descriptor 
    if(0 > bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
        close(listenfd);
        R krr("bind failed");
    };

    R sd1(listenfd, callback);
 }; 

// Client side functions - one shot UDP 
// Negative ports -> "async" -> Doesn't check if msg successfully sends
K sendOneShotUDP(K host, K port, K msg) {

    // Check for host/port types
    // Keep ports by default int-types
    Pkrr(host->t != KC && host->t != -KS, "host string/symbol type");
    Pkrr(port->t != -KI, "port int type");
    Pkrr(msg->t != KC, "msg string type");
    
    Pkrr(msg->n > BUFFERLENGTH, "Above Safe UDP Payload");

    I sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    Pkrr(0 > sockfd, "socket creation failed");
  
    bzero(&servaddr, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(abs(port->i)); 
    servaddr.sin_addr.s_addr = inet_addr(kdb2str(host)); 
    
    // The first packet on a new socket will always succeed 
    sendto(sockfd, kdb2str(msg), msg->n, MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));

    // Receive UDP ACK Msg
    // Only check if port is positive/zero
    if(0 <= port->i) {
        if(1 != checkSocketTimeout(sockfd)) {
            close(sockfd);
            R krr("Failed to send UDP Msg");
        };
    };

    close(sockfd); 
    R (K) 0;
 };

// Client side functions - Connect to UDP
K connectUDP(K host, K port) {

    Pkrr(host->t != KC && host->t != -KS, "host string/symbol type");
    Pkrr(port->t != -KI, "port int type");
    
    Pkrr(0 >= port->i, "port must be above zero");

    I sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    Pkrr(0 > sockfd, "socket creation failed");
  
    bzero(&servaddr, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port->i); 
    servaddr.sin_addr.s_addr = inet_addr(kdb2str(host)); 
    
    // bind server address to socket descriptor 
    // Connect will always succeed
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    // Check if the sockfd is listening in the first place
    if(-1 == sendto(sockfd, " ", 1, MSG_CONFIRM, NULL, 0)) {
        close(sockfd);
        R krr("UDP connect failed");
    };

    // Receive UDP ACK Msg
    if(1 != checkSocketTimeout(sockfd)) {
        close(sockfd);
        R krr("UDP connect failed");
    };

    R ki(sockfd);
 };

// Client side functions --> send UDP Msg
// Allow for async calls (negative i) --> doesn't check for return acknowledgment msg
K sendUDPMsg(K sockfd, K msg) {

    Pkrr(msg->t != KC, "msg string type");
    Pkrr(sockfd->t != -KI, "sockfd int type");

    Pkrr(msg->n > BUFFERLENGTH, "Above Safe UDP Payload");    

    int socketfd = abs(sockfd->i);

    Pkrr(0 == socketfd, "zero port");

    if(-1 == sendto(socketfd, kdb2str(msg), msg->n, MSG_CONFIRM, NULL, 0)) {
        close(socketfd);
        R krr("Failed to send UDP Msg");
    };

    // Receive UDP ACK Msg -> only if sync calls
    if(0 < sockfd->i) {
        if(1 != checkSocketTimeout(socketfd)) {
            close(socketfd);
            R krr("Failed to send UDP Msg");
        }
    }

    R (K) 0;
 };