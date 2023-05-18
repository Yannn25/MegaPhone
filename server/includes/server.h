#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_CLIENT 30

typedef struct client_s {
    int socket;
    int id;
    char *name;
    int *team;
    char *buffer;
    int valread;
    int actual_cmd;
} client_t;

typedef struct server_s {
    int socket;
    int port;
    int addrlen;
    fd_set readfds;
    struct sockaddr_in address;
    client_t *clients;
    int talk_sock;
    int nb_clients;
    char **usernames;
    int user_nb;
} server_t;

int server();

#endif /* !SERVER_H_ */