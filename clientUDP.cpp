
#include <iostream>
#include <string>
#include <boost/asio.hpp>

#define KXVER 3
#include "k.h"

// Safe UDP Payload
#define BUFFERLENGTH 500

// g++ -shared -fPIC clientUDP.cpp -pthread -lboost_system -o clientUDP.so
extern "C" {

Z __inline S c2s(S s,J n){S r=(S)malloc(n+1);R r?memcpy(r,s,n),r[n]=0,r:(S)krr((S)"wsfull");}

static S kdb2str(K kdbStr) {return kdbStr->t == -KS ? kdbStr->s : c2s((S) kC(kdbStr), kdbStr->n);}

// Modified from https://www.bookstack.cn/read/boost-asio-network-programming-little-book/posts-udp-communication.md
K sendUDP(K host, K port, K msg) {

    // Check for host/port types
    // Keep ports by default int-types
    P(host->t != KC && host->t != -KS, krr((S) "host string/symbol type"));
    P(port->t != -KI, krr((S) "port int type"));
    P(msg->t != KC, krr((S) "msg string type"));
    P(msg->n > BUFFERLENGTH, krr((S) "Above Safe UDP Payload"));

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::udp::socket socket{io_context};
        socket.open(boost::asio::ip::udp::v4());
        socket.send_to(
            boost::asio::buffer(std::string(kdb2str(msg))),
            boost::asio::ip::udp::endpoint{boost::asio::ip::make_address(std::string(kdb2str(host))), static_cast<ushort>(port->i)}
        );
        socket.close();
    } catch (std::exception& e) { 
        R krr(strdup(e.what()));
    }
    R (K) 0;
}

}




