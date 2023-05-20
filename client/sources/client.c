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
    free(str);
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
    free(msg);
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
    free(buff);
}

void post_message(client_t *client, int thread_num)
{
    char *message = malloc(sizeof(char) * 1024);

    strcat(message, "1:");             // la commande pour poster un message
    strcat(message, itoa(client->id)); // ajouter l'id du client
    strcat(message, ":");
    strcat(message, itoa(thread_num)); // ajouter le numéro du fil
    strcat(message, ":");
    strcat(message, client->buffer); // ajouter le message du client

    // envoyer le message
    if (send_message(message, client->socket, client) < 0)
    {
        printf("Erreur lors de l'envoi du message\n");
    }

    free(message);
}

void menu(client_t *client)
{
    char choice[1024];
    while (1)
    {
        printf("1. Poster un billet\n");
        printf("2. demander la liste des billets\n");
        printf("3. s'abonner à un fil\n");
        printf("4. Quitter\n");
        printf("Choisissez une option: ");
        fgets(choice, 1024, stdin);
        choice[strcspn(choice, "\n")] = 0;

        if (strcmp(choice, "1") == 0)
        {
            printf("Ecrivez votre billet: ");
            fgets(client->buffer, 1024, stdin);
            client->buffer[strcspn(client->buffer, "\n")] = 0;

            // demande le numéro du fil
            printf("Entrer le numéro du fil: ");
            char thread_num_str[50];
            fgets(thread_num_str, 50, stdin);
            int thread_num = atoi(thread_num_str);

            // appeler la fonction post_message()
            post_message(client, thread_num);
        }
        else if (strcmp(choice, "2") == 0)
        {
            printf("Demande de la liste des billets\n");
        }
        else if (strcmp(choice, "3") == 0)
        {
            printf("Demande d'abonnement à un fil\n");
        }
        else if (strcmp(choice, "4") == 0)
        {
            close(client->socket);
            exit(0);
        }
        else
        {
            printf("Choix invalide. Essayez encore.\n");
        }
    }
}

int command_handler(client_t *client)
{
    menu(client);
}

int connect_to_server(int port, char *ip)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;
    serv_addr = (struct sockaddr_in){.sin_family = AF_INET, .sin_port = htons(port)};
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
    {
        close(client.socket);
        return -1;
    }

    command_handler(&client);
    printf("Connected to server\n");

    while (1)
    {
        client.buffer = memset(client.buffer, 0, 1024);
        client.valread = read(0, client.buffer, 1024);
        printf("%s", client.buffer);
        command_handler(&client);
    }

    close(client.socket); // This should be placed here after the infinite loop.

    return 0;
}

int client()
{
    connect_to_server(7777, "127.0.0.1");
    return 0;
}