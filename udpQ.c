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

#define Pkrr(x,y) P(x,krr(y)) 
#define isSync(x) (0<x->i)

// Max Safe UDP Payload (to avoid reassembly issue)
#define BUFFERLENGTH 1400

// gcc -shared -fPIC -o udpQ.so udpQ.c

Z __inline S c2s(S s,J n){S r=(S)malloc(n+1);R r?memcpy(r,s,n),r[n]=0,r:(S)krr((S)"wsfull");}

// Note kG returns (S)
Z S kdb2str(K kdbStr) {R (-KS==kdbStr->t) ? kdbStr->s : c2s(kG(kdbStr), kdbStr->n);}

// Set socket timeouts
// https://code.kx.com/q/interfaces/c-client-for-q/#socket-timeouts
V sst(I sockfd, I timeout) {
  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = timeout%1000;
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (S)&tv, sizeof(tv)); // Send timeout
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (S)&tv, sizeof(tv)); // Recv timeout
 };

K setSocketTimeout(K sockfd, K timeout) {
  Pkrr(-KI != sockfd->t, "sockfd int type");
  Pkrr(!(-KI == timeout->t || -KJ == timeout->t), "timeout int/long type");
  I timeoutVal = (I) (-KJ==timeout->t) ? timeout->j : timeout->i;
  P(0 >= timeoutVal, (K)0);
  sst(sockfd->i, timeoutVal);  
  R (K)0;
 };

// Modified from 
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/?ref=rp 
// https://stackoverflow.com/questions/16163260/setting-timeout-for-recv-fcn-of-a-udp-socket
K callback(I listenfd) {
    
  // Receive message from server
  char buffer[BUFFERLENGTH]; 
  bzero(&buffer, sizeof(buffer));

  struct sockaddr_in cliaddr;
  bzero(&cliaddr, sizeof(cliaddr));
  I len = sizeof(cliaddr);

  I n = recvfrom(listenfd, (S)buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);  
    
  // Process message and serialise it
  K recvMsg = k(0, "@[value;;\"Error: '\",]", kp(buffer), (K)0);
  K callbackMsg = b9(3, recvMsg);
  r0(recvMsg);

  if(callbackMsg->n > BUFFERLENGTH) {
    r0(callbackMsg);
    K errstr = kpn("'Exceeds Safe UDP Payload",25); K serrstr = b9(3,errstr);
    sendto(listenfd, kG(serrstr), serrstr->n, MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);
    r0(errstr); r0(serrstr);
    R krr("Exceeds Safe UDP Payload");
  };

  if(-1 == sendto(listenfd, kG(callbackMsg), callbackMsg->n, MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len)) {
    r0(callbackMsg);
    R krr("Error sending callback msg");
  };
  r0(callbackMsg);

  R (K)0;
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
    R orr("bind failed");
  };

  R sd1(-listenfd, callback);
 }; 

// Client side functions - Connect to UDP
K connectUDP(K host, K port) {

  Pkrr(host->t != KC && host->t != -KS, "host string/symbol type");
  Pkrr(port->t != -KI, "port int type");

  Pkrr(0 >= port->i, "port must be above zero");

  I sockfd;
  
  // Creating socket file descriptor
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  Pkrr(0 > sockfd, "socket creation failed");

  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));

  // Filling server information 
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port->i);
  servaddr.sin_addr.s_addr = inet_addr(kdb2str(host));
    
  // bind server address to socket descriptor 
  // Connect will always succeed
  if(0 > connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
    close(sockfd);
    R orr("UDP connect failed");        
  };

  // Set socket timeout to 5s by default
  sst(sockfd, 5);

  R ki(sockfd);
 };

// Client side functions --> send UDP Msg
K sendUDP(K sockfd, K msg) {

  Pkrr(!(KC == msg->t), "msg string type");
  Pkrr(sockfd->t != -KI, "sockfd int type");
  Pkrr(!isSync(sockfd), "only sync calls");

  I socketfd = abs(sockfd->i);

  if(-1 == sendto(socketfd, kG(msg), msg->n, MSG_CONFIRM, NULL, 0)) {
    close(socketfd);
    R orr("Failed to send UDP Msg");
  };

  if(isSync(sockfd)) {
    char buffer[BUFFERLENGTH];
    bzero(&buffer, sizeof(buffer));
    I n = recvfrom(socketfd, (S)buffer, sizeof(buffer), MSG_WAITALL, NULL, NULL);
    if(-1 == n) {
      close(socketfd);
      R orr("Error receiving message, closing socket!");
    };
      
    // Copy to k struct up to maximum buffer size
    K r = ktn(KG, n);
    memcpy(kG(r), buffer, n);
    K res = d9(r);
    r0(r);
      
    // Deserialise msg
    R res;
  };

  R (K)0;
 };

// Client side functions -> send one shot UDP Msg
K sendOneShotUDP(K host, K port, K msg, K timeout) {
  K sockfd = connectUDP(host, port);
  setSocketTimeout(sockfd, timeout);
  K r = sendUDP(sockfd, msg);
  close(sockfd->i);
  r0(sockfd);
  R r;
 };