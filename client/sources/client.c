#include <stdio.h>
#include "client.h"
#include "color.h"

char *itoa(int num)
{
    if (num == 0)
    {
        char *ret = malloc(2);
        strcpy(ret, "0");
        return ret;
    }
    int len = 0, temp = num;
    while (temp)
    {
        len++;
        temp /= 10;
    }
    char *ret = malloc(len + 1);
    for (int i = len - 1; i >= 0; i--)
    {
        ret[i] = (num % 10) + '0';
        num /= 10;
    }
    ret[len] = '\0';
    return ret;
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
    printf("%d %d\n", client->cmd_nb, client->id);
    sprintf(msg, "%d:%d:%s", client->cmd_nb, client->id, message);
    if (write(socket, msg, strlen(msg)) < 0)
        return -1;
    return 0;
}

int connection(client_t *client)
{
    char *buff = malloc(sizeof(char) * 1024);
    int valread = 0;
    valread = read(client->socket, buff, 1024);
    if (valread < 0)
    {
        perror("read in connection");
        return -1;
    }
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

int connect_express(int port, char *ip, client_t *client)
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
    client->socket = sock;
    return 0;
}

void post_message(client_t *client, int thread_num)
{
    char *message = calloc(1024, sizeof(char));
    client->cmd_nb = 1;
    strcat(message, itoa(thread_num));
    strcat(message, ":");
    strcat(message, client->buffer);
    // envoyer le message
    printf("%d - %d - %s\n", client->cmd_nb, client->id, message);
    connect_express(client->port, client->ip, client);
    printf("%d - %d - %s\n", client->cmd_nb, client->id, message);
    if (send_message(message, client->socket, client) < 0)
    {
        printf("Erreur lors de l'envoi du message\n");
    }
    char *buff = malloc(sizeof(char) * 1024);
    read(client->socket, buff, 1024);
    printf("%s\n", buff);
}

void get_last_n_messages(int n, int thread_id, client_t *client)
{
    char request[1024];
    client->cmd_nb = 2;

    // Créer une requête pour récupérer les n derniers messages du fil d'identifiant thread_id
    sprintf(request, "GET_LAST_N_MESSAGES:%d:%d\n", n, thread_id);

    // Envoyer la requête au serveur
    if (write(client->socket, request, strlen(request)) < 0)
    {
        printf("Failed to send request to server\n");
        return;
    }
    {
        printf("Failed to send request to server\n");
        return;
    }

    // Réception de la réponse du serveur
    int num_messages = 0;
    if (read(client->socket, &num_messages, sizeof(int)) < 0)
    {
        printf("Failed to read from server\n");
        return;
    }

    // Réception des messages
    for (int i = 0; i < num_messages; ++i)
    {
        char message[1024];
        if (read(client->socket, message, sizeof(message)) < 0)
        {
            printf("Failed to read from server\n");
            return;
        }

        printf("Received message: %s\n", message);
    }
}

void menu(client_t *client)
{
    char *choice = NULL;
    size_t len = 0;
    while (1)
    {
        printf("1. Poster un billet\n");
        printf("2. demander la liste des billets\n");
        printf("3. s'abonner à un fil\n");
        printf("4. Quitter\n");
        printf("Choisissez une option: ");
        getline(&choice, &len, stdin);
        printf("Vous avez choisi l'option %s", choice);

        if (strncmp(choice, "1", 1) == 0)
        {
            printf("Ecrivez votre billet: \n");
            if (read(0, client->buffer, 1024) < 0)
            {
                perror("read failed in menu");
                return;
            }
            // getline(&client->buffer, &len, stdin);

            // demande le numéro du fil
            printf("Entrer le numéro du fil: \n");
            char thread_num_str[1024] = {0};
            if (read(0, thread_num_str, 1024) < 0)
            {
                perror("read failed in menu");
                return;
            }
            // getline(&thread_num_str, &len, stdin);
            int thread_num = atoi(thread_num_str);

            // appeler la fonction post_message()
            post_message(client, thread_num);
        }
        else if (strncmp(choice, "2", 1) == 0)
        {
            printf("Liste des billets :\n");

            // demander le numéro du fil
            printf("Entrer le numéro du fil: \n");
            char thread_num_str[1024] = {0};
            if (read(0, thread_num_str, 1024) < 0)
            {
                perror("read failed in menu");
                return;
            }
            // getline(&thread_num_str, &len, stdin);
            int thread_num = atoi(thread_num_str);

            // demander combien de messages à afficher
            printf("Combien de messages voulez-vous afficher? \n");
            char num_messages_str[1024] = {0};
            if (read(0, num_messages_str, 1024) < 0)
            {
                perror("read failed in menu");
                return;
            }
            // getline(&num_messages_str, &len, stdin);
            int num_messages = atoi(num_messages_str);

            // appeler la fonction get_last_n_messages()
            get_last_n_messages(num_messages, thread_num, client);
        }
        else if (strncmp(choice, "3", 1) == 0)
        {
            printf("Demande d'abonnement à un fil\n");
        }
        else if (strncmp(choice, "4", 1) == 0)
        {
            free(choice);
            close(client->socket);
            exit(0);
        }
        else
        {
            printf("Choix invalide. Essayez encore.\n");
        }
        free(choice);
        choice = NULL;
        len = 0;
    }
}

int command_handler(client_t *client)
{
    menu(client);
    return 0;
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

    send_message("Connection", client.socket, &client);
    if (connection(&client) < 0)
    {
        close(client.socket);
        return -1;
    }
    printf("Connected to server\n");
    close(client.socket); // This should be placed here after the infinite loop.

    while (1)
    {
        command_handler(&client);
    }

    return 0;
}

int client()
{
    connect_to_server(7778, "127.0.0.1");
    return 0;
}