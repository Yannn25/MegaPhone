#include <stdio.h>
#include "client.h"

char *itoa(int nb)
{
    char *str = malloc(sizeof(char) * 1024);
    int i = 0;
    int j = 0;
    int k = 0;

    if (nb == 0)
        return "0";
    while (nb > 0)
    {
        str[i] = nb % 10 + '0';
        nb /= 10;
        i++;
    }
    str[i] = '\0';
    for (j = 0, k = i - 1; j < k; j++, k--)
    {
        str[j] ^= str[k];
        str[k] ^= str[j];
        str[j] ^= str[k];
    }
    return str;
}

char **string_to_tab(char *str, char sep)
{
    char **tab = malloc(sizeof(char *) * 1024);
    int i = 0;
    int j = 0;
    int k = 0;

    tab[i] = malloc(sizeof(char) * 1024);
    while (str[j] != '\0')
    {
        if (str[j] == sep)
        {
            tab[i][k] = '\0';
            i++;
            k = 0;
            tab[i] = malloc(sizeof(char) * 1024);
        }
        else
        {
            tab[i][k] = str[j];
            k++;
        }
        j++;
    }
    tab[i][k] = '\0';
    tab[i + 1] = NULL;
    return tab;
}

int send_message(char *message, int socket, client_t *client)
{
    char *msg = malloc(sizeof(char) * 1024);
    strcat(msg, itoa(client->cmd_nb));
    strcat(msg, ":");
    strcat(msg, itoa(client->id));
    strcat(msg, ":");
    strcat(msg, message);
    if (write(socket, msg, strlen(msg)) < 0)
        return -1;
    return 0;
}

int connection(client_t *client)
{
    char *buff = malloc(sizeof(char) * 1024);
    int valread = 0;
    valread = read(client->socket, buff, 1024);
    if (strcmp(buff, "Etes vous inscrit ?(yes/no)\n") == 0)
    {
        printf("%s", buff);
        buff = memset(buff, 0, 1024);
        valread = read(0, buff, 1024);
        send_message(buff, client->socket, client);
        buff = memset(buff, 0, 1024);
        valread = read(client->socket, buff, 1024);
        if (strcmp(buff, "Entrez votre username\n") == 0)
        {
            printf("%s", buff);
            buff = memset(buff, 0, 1024);
            valread = read(0, buff, 1024);
            send_message(buff, client->socket, client);
            buff = memset(buff, 0, 1024);
            valread = read(client->socket, buff, 1024);
            printf("%s", buff);
            if (strncmp(buff, "Vous etes connecte", 18) == 0)
            {
                char **tab = string_to_tab(buff, ':');
                client->id = atoi(tab[1]);
                printf("Votre id est %d\n", client->id);
                return 0;
            }
            else
                return -1;
        }
        else
            return -1;
    }
    else
        return -1;
}

int command_handler(client_t *client)
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;
    serv_addr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_port = htons(client->port)};
    if (inet_pton(AF_INET, client->ip, &serv_addr.sin_addr) <= 0)
        return -1;
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return -1;
    send_message("connection", sock, client);
}

int connect_to_server(int port, char *ip)
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;
    serv_addr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_port = htons(port)};
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
        return -1;
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return -1;
    client_t client = {
        .socket = sock,
        .buffer = malloc(sizeof(char) * 1024),
        .valread = 0,
        .serv_buffer = malloc(sizeof(char) * 1024),
        .serv_valread = 0,
        .name = malloc(sizeof(char) * 1024),
        .address = serv_addr,
        .addrlen = sizeof(serv_addr),
        .readfds = (fd_set){0},
        .id = 0,
        .cmd_nb = 0,
        .ip = strdup(ip),
        .port = port};
    if (connection(&client) < 0)
        return -1;
    printf("Connected to server\n");
    close(client.socket);
    while (1)
    {
        client.buffer = memset(client.buffer, 0, 1024);
        client.valread = read(0, client.buffer, 1024);
        printf("%s", client.buffer);
        command_handler(&client);
    }
    return 0;
}

int client()
{
    connect_to_server(7777, "127.0.0.1");
    return 0;
}