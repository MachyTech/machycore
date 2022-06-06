#ifndef SOCKETDEV_H
#define SOCKETDEV_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

//#define DEBUG

class socket_programming{
    public:
        const char* ip_addr;
        const char* port;
        int bytes_received;
        char read[8192];
        struct addrinfo hints;
        struct addrinfo *peer_address;
        SOCKET socket_peer;
        socket_programming(std::string x, std::string y){
            ip_addr = x.c_str();
            port = y.c_str();
            init_socket();
            print_addr();
            create();
        }
        void init_socket(void);
        void print_addr(void);
        void create(void);
        void make_connection(void);
        void wait_for_connection(void);
        char * run(void);
        ~socket_programming(void);
};

#endif