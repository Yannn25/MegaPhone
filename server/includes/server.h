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
#include <pthread.h>

#define MAX_CLIENT 30
#define MAX_THREADS 100
#define BUF_LEN 1024
#define DEFAULT_MULTI_PORT 7000

typedef struct Message
{
    int id;
    int author_id;
    char *content;
    struct Message *next;
} Message;

typedef struct Thread
{
    int id;
    char *name;
    Message *messages;
} Thread;
typedef struct client_s
{
    int socket;
    int id;
    char *name;
    int *team;
    char *buffer;
    int valread;
    int actual_cmd;
} client_t;

typedef struct server_s
{
    int socket;
    int port;
    int addrlen;
    fd_set readfds;
    struct sockaddr_in6 address;
    client_t *clients;
    int talk_sock;
    int nb_clients;
    char **usernames;
    int user_nb;
    Thread threads[MAX_THREADS];
    int nb_threads;
} server_t;

typedef struct format_message_s {
    int CODEREQ;
    int ID;
    int NUMFIL;
    int NB;
    int LENDATA;
    char *DATA;
} format_message_t;

int server();

#endif /* !SERVER_H_ */