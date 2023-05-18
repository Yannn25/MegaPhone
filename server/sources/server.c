#include <stdio.h>
#include "server.h"

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

int send_message(char *msg, client_t client)
{
    if (write(client.socket, msg, strlen(msg)) < 0)
        return -1;
    return 0;
}

int server_client_activity(server_t *server)
{
    client_t *client = NULL;

    for (int i = 0; i < server->nb_clients; i++)
    {
        client = &server->clients[i];
        if (FD_ISSET(client->socket, &server->readfds))
        {
            client->valread = read(client->socket, client->buffer, 1024);
            switch (client->valread)
            {
            case -1:
                return -1;
            case 0:
                close(client->socket);
                client->socket = -1;
                break;
            default:
                printf("Received from %d: %s\n", client->socket, client->buffer);
                client->buffer = malloc(sizeof(char) * 1024);
                break;
            }
        }
    }
    return 0;
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

int inscription(server_t *server, char *pseudo)
{
    int i = 0;
    for (i = 0; server->usernames[i] != NULL; ++i)
    {
        if (strcmp(server->usernames[i], pseudo) == 0)
            return -1;
    }
    char **tab = malloc(sizeof(char *) * server->user_nb + 1);
    for (int j = 0; j < server->user_nb; ++j)
    {
        tab[j] = malloc(sizeof(char) * 1024);
        tab[j] = server->usernames[j];
    }
    tab[server->user_nb] = malloc(sizeof(char) * 1024);
    strcpy(tab[server->user_nb], pseudo);
    tab[server->user_nb + 1] = NULL;
    server->usernames = malloc(sizeof(char *) * server->user_nb + 1);
    for (int j = 0; j < server->user_nb + 1; ++j)
    {
        server->usernames[j] = malloc(sizeof(char) * 1024);
        server->usernames[j] = tab[j];
    }
    server->usernames[server->user_nb + 1] = NULL;
    server->user_nb++;
    return 0;
}

int check_connection(server_t *server, char *pseudo)
{
    for (int i = 0; i < server->user_nb; ++i)
    {
        if (strcmp(server->usernames[i], pseudo) == 0)
            return 0;
    }
    return -1;
}

int connection(server_t *server, int i)
{
    send_message("Etes vous inscrit ?(yes/no)\n", server->clients[i]);
    read(server->clients[i].socket, server->clients[i].buffer, 1024);
    printf("%s\n", server->clients[i].buffer);
    char **buf = string_to_tab(server->clients[i].buffer, ':');
    printf("%s - %s - %s\n", buf[0], buf[1], buf[2]);
    if (strncmp(buf[2], "yes", 3) == 0)
    {
        send_message("Entrez votre username\n", server->clients[i]);
        read(server->clients[i].socket, server->clients[i].buffer, 1024);
        printf("%s\n", server->clients[i].buffer);
        buf = string_to_tab(server->clients[i].buffer, ':');
        char *msg = malloc(sizeof(char) * 1024);
        if (check_connection(server, buf[2]) == -1)
        {
            msg = strcat(msg, "Vous n'etes pas inscrit\n");
            send_message(msg, server->clients[i]);
            return -1;
        }
        msg = strcat(msg, "Vous etes connecte:");
        msg = strcat(msg, itoa(server->clients[i].id));
        msg = strcat(msg, "\n");
        send_message(msg, server->clients[i]);
    }
    else if (strncmp(buf[2], "no", 2) == 0)
    {
        send_message("Entrez votre username\n", server->clients[i]);
        read(server->clients[i].socket, server->clients[i].buffer, 1024);
        printf("%s\n", server->clients[i].buffer);
        buf = string_to_tab(server->clients[i].buffer, ':');
        char *msg = malloc(sizeof(char) * 1024);
        if (inscription(server, buf[2]) == -1)
        {
            msg = strcat(msg, "Pseudo incorrect\n");
            send_message(msg, server->clients[i]);
            return -1;
        }
        msg = strcat(msg, "Vous etes connecte:");
        msg = strcat(msg, itoa(server->clients[i].id));
        msg = strcat(msg, "\n");
        send_message(msg, server->clients[i]);
    }
    else
    {
        send_message("Error\n", server->clients[i]);
        return -1;
    }
    server->clients[i].buffer = malloc(sizeof(char) * 1024);
}

int server_activity(server_t *server)
{
    int activity = 0;
    int new_socket = -1;

    if (FD_ISSET(server->socket, &server->readfds))
    {
        if ((new_socket = accept(server->socket, (struct sockaddr *)&server->address, (socklen_t *)&server->addrlen)) < 0)
            return -1;
        printf("New connection, socket fd is %d, ip is : %s, port : %d\n", new_socket, inet_ntoa(server->address.sin_addr), ntohs(server->address.sin_port));
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            if (server->clients[i].socket == -1)
            {
                server->clients[i].socket = new_socket;
                printf("Adding to list of sockets as %d\n", i);
                server->nb_clients++;
                server->clients[i].buffer = malloc(sizeof(char) * 1024);
                server->clients[i].valread = 0;
                server->clients[i].name = malloc(sizeof(char) * 1024);
                server->clients[i].actual_cmd = 0;
                server->clients[i].id = i + 1;
                printf("%d, %d\n", server->clients[i].socket, server->nb_clients);
                // if id != 0 -/> connection
                if (connection(server, i) != 0)
                {
                    close(server->clients[i].socket);
                    server->clients[i].socket = -1;
                    server->nb_clients--;
                }
                break;
            }
        }
    }
    if (server_client_activity(server) != 0)
        return -1;
    return 0;
}

int server_check_talking(server_t *server)
{
    for (int i = 0; i < server->nb_clients; i++)
    {
        if (server->clients[i].socket > 0)
            FD_SET(server->clients[i].socket, &server->readfds);
        if (server->clients[i].socket > server->talk_sock)
            server->talk_sock = server->clients[i].socket;
    }
    return 0;
}

int server_run(server_t *server)
{
    int activity = 0;
    int new_socket = -1;

    while (1)
    {
        FD_ZERO(&server->readfds);
        FD_SET(server->socket, &server->readfds);
        server->talk_sock = server->socket;
        if (server_check_talking(server) != 0)
            return -1;
        if ((activity = select(server->talk_sock + 1, &server->readfds, NULL, NULL, NULL)) < 0 && errno != EINTR)
            return -1;
        if (server_activity(server) != 0)
            return -1;
    }
}

server_t create_server(int port)
{
    server_t server = {.port = port};
    server.usernames = malloc(sizeof(char *) * 1);
    server.usernames[0] = "NULL";
    server.user_nb = 0;
    if ((server.socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit(84);
    server.address = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)};
    server.addrlen = sizeof(server.address);
    if (bind(server.socket, (struct sockaddr *)&server.address, server.addrlen) < 0)
        exit(84);
    if (listen(server.socket, 3) < 0)
        exit(84);
    server.clients = malloc(sizeof(client_t) * MAX_CLIENT);
    for (int i = 0; i < MAX_CLIENT; i++)
        server.clients[i].socket = -1;
    server.nb_clients = 0;
    return server;
}

int server()
{
    server_t server = create_server(7777);
    printf("Server is running on port %d:%d\n", server.port, server.socket);
    if (server_run(&server) != 0)
        return -1;
    return 0;
}