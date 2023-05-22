#include <stdio.h>
#include "client.h"
#include "color.h"

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
    printf("%d %d\n", client->cmd_nb, client->id);
    sprintf(msg, "%d:%d:%s", client->cmd_nb, client->id, message);
    if (write(socket, msg, strlen(msg)) < 0)
        return -1;
    return 0;
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
/*Formatage avant envoi id et codereq*/
uint16_t formatage_entete_envoi(entete_t format) {
    //Formattage coté client
    format.id = format.id << 5;
    uint16_t entete = format.codereq + format.id;
    entete = htons(entete); // big endian
    return entete;
}

void post_message(client_t *client, int thread_num)
{
    char *message = malloc(sizeof(char) * BUF_LEN);
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
    char *buff = malloc(sizeof(char) * BUF_LEN);
    read(client->socket, buff, BUF_LEN);
    printf("%s\n", buff);
}


void abonnement_fil(client_t *client, int num) {
    char *message = malloc(sizeof(char) * BUF_LEN);
    strcat(message, "3:");           
    strcat(message, itoa(client->id)); 
    strcat(message, ":");
    strcat(message, itoa(num));
    strcat(message, ":0:0");
    // Envoye du message d'abonnement au serveur
    send_message(message, client->socket, client); 
    //Reception de la réponse du serveur
    char *buff = malloc(sizeof(char) * BUF_LEN);
    read(client->socket, buff, BUF_LEN);
    printf("%s\n", buff);
    string_to_tab(buff,':');
    // Enregistrer l'adresse d'abonnement
    client->multi_ip = strdup(buff[1]);
    client->multi_port = atoi(buff[3]);  
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
                printf("%sPoster un billet%s\nEcrivez votre billet: ", GREEN, RESET);
                fgets(client->buffer, BUF_LEN, stdin);
                client->buffer[strcspn(client->buffer, "\n")] = 0;

                // demande le numéro du fil
                printf("Entrer le numéro du fil: ");
                char thread_num_str[REP_LEN];
                fgets(thread_num_str, REP_LEN, stdin);
                int thread_num = atoi(thread_num_str);

                // appeler la fonction post_message()
                post_message(client, thread_num);
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
                sprintf(message, "3:%d:%d:0:%d", client->id, f, n);

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
                printf("\n%sDemande d'abonnement à un fil%s\nEntrer le numéro du fil: ", BLUE, RESET);
                char rep[REP_LEN];
                fgets(rep, REP_LEN, stdin);
                int fil_num_abo = atoi(rep);

                abonnement_fil(client, fil_num_abo);
                break;

            case 4:
                printf("\n%sAjouter un fichier%s\n", MAGENTA, RESET);
                break;

            case 5:
                printf("\n%sTélécharger un fichier%s\n", YELLOW, RESET);
                break;

            case 6:
                printf("%sFIN DE CONNEXION%s\n", RED, RESET);
                close(client->socket);
                exit(0);
                break;

            default:
                printf("%sChoix invalide. Essayez encore.%s\n", RED, RESET);
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
    connect_to_server(9494, "127.0.0.1");
    return 0;
}