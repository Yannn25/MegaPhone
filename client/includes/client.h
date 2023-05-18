

#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int client();

typedef struct client_s {
    int socket;
    char *name;
    char *buffer;
    int valread;
    char *serv_buffer;
    int serv_valread;
    int addrlen;
    struct sockaddr_in address;
    fd_set readfds;
    int id;
    int cmd_nb;
    char *ip;
    int port;
} client_t;

#endif

