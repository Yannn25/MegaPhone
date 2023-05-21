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
#include <pthread.h>

#define BUF_LEN 1024
#define REP_LEN 50

int client();
typedef struct client_s
{
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
    char *multi_ip;
    int multi_port;
} client_t;

typedef struct subscribe_message_s {
    int CODEREQ;
    int ID;
    int NUMFIL;
    int NB;
    int LENDATA;
    char *DATA;
} subscribe_message_t;

typedef struct entete {
    uint16_t codereq;
    uint16_t id;
} entete_t;

#endif
