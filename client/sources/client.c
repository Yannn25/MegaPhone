#include <stdio.h>
#include "client.h"

char *itoa(int nb)
{
    char *str = malloc(sizeof(char) * BUF_LEN);
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
    char **tab = malloc(sizeof(char *) * BUF_LEN);
    int i = 0;
    int j = 0;
    int k = 0;

    tab[i] = malloc(sizeof(char) * BUF_LEN);
    while (str[j] != '\0')
    {
        if (str[j] == sep)
        {
            tab[i][k] = '\0';
            i++;
            k = 0;
            tab[i] = malloc(sizeof(char) * BUF_LEN);
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
    char *msg = malloc(sizeof(char) * BUF_LEN);
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
    char *buff = malloc(sizeof(char) * BUF_LEN);
    int valread = 0;
    valread = read(client->socket, buff, BUF_LEN);
    if (strcmp(buff, "Etes vous inscrit ?(yes/no)\n") == 0)
    {
        printf("%s", buff);
        buff = memset(buff, 0, BUF_LEN);
        valread = read(0, buff, BUF_LEN);
        send_message(buff, client->socket, client);
        buff = memset(buff, 0, BUF_LEN);
        valread = read(client->socket, buff, BUF_LEN);
        if (strcmp(buff, "Entrez votre username\n") == 0 || strcmp(buff, "Choissisez  un pseudo\n") == 0)
        {
            printf("%s", buff);
            buff = memset(buff, 0, BUF_LEN);
            valread = read(0, buff, BUF_LEN);
            send_message(buff, client->socket, client);
            buff = memset(buff, 0, BUF_LEN);
            valread = read(client->socket, buff, BUF_LEN);
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
    char *message = malloc(sizeof(char) * BUF_LEN);

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

void menu(client_t *client) {
    char choice[BUF_LEN];
    while (1) {
        printf("1. Poster un billet\n");
        printf("2. demander la liste des billets\n");
        printf("3. s'abonner à un fil\n");
        printf("4. Ajouter un fichier\n");
        printf("5. Télécharger un fichier\n");
        printf("6. Quitter\n");
        printf("Choisissez une option: ");
        fgets(choice, BUF_LEN, stdin);
        choice[strcspn(choice, "\n")] = 0;

        switch (atoi(choice)) {
            case 1:
                printf("Poster un billet\nEcrivez votre billet: ");
                fgets(client->buffer, BUF_LEN, stdin);
                client->buffer[strcspn(client->buffer, "\n")] = 0;

                // demande le numéro du fil
                printf("Entrer le numéro du fil: ");
                char thread_num_str[50];
                fgets(thread_num_str, 50, stdin);
                int thread_num = atoi(thread_num_str);

                // appeler la fonction post_message()
                post_message(client, thread_num);
                //reception du message
                read(client->socket, client->buffer, BUF_LEN);
                break;

            case 2:
                printf("Demande de la liste des billets\n");

                // Request the list of last n tickets
                int n;
                int f;
                printf("Entrez le nombre de billets à afficher: ");
                scanf("%d", &n);
                printf("Entrez le numéro du fil (0 for all threads): ");
                scanf("%d", &f);

                // Construct the request message
                char message[BUF_LEN];
                sprintf(message, "3:%d:%d:0:", client->id, f, n);

                // Send the request message to the server
                send_message(message, client->socket, client);

                // Receive and print the response messages from the server
                int num_messages;
                read(client->socket, &num_messages, sizeof(int));
                printf("Received %d messages:\n", num_messages);
                for (int i = 0; i < num_messages; i++) {
                    read(client->socket, client->buffer, BUF_LEN);
                    printf("%s\n", client->buffer);
                }
                break;
            case 3:
                printf("Demande d'abonnement à un fil\n");
                break;

            case 4:
                printf("Ajouter un fichier\n");
                break;

            case 5:
                printf("Télécharger un fichier\n");
                break;

            case 6:
                close(client->socket);
                exit(0);
                break;

            default:
                printf("Choix invalide. Essayez encore.\n");
                break;
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
        .buffer = malloc(sizeof(char) * BUF_LEN),
        .valread = 0,
        .serv_buffer = malloc(sizeof(char) * BUF_LEN),
        .serv_valread = 0,
        .name = malloc(sizeof(char) * BUF_LEN),
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
        client.buffer = memset(client.buffer, 0, BUF_LEN);
        client.valread = read(0, client.buffer, BUF_LEN);
        printf("%s", client.buffer);
        command_handler(&client);
    }

    close(client.socket); // This should be placed here after the infinite loop.

    return 0;
}

int client()
{
    connect_to_server(9493, "127.0.0.1");
    return 0;
}