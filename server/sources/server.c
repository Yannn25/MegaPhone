#include <stdio.h>
#include "server.h"
#include "color.h"
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
                free(client->buffer); // Libérer la mémoire allouée pour client->buffer
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
    server->usernames = malloc(sizeof(char *) * (server->user_nb + 1));
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

int check_pseudo_length(char *pseudo)
{
    int length = strlen(pseudo);
    if (length <= 3)
    {
        return -1; // Pseudo trop court
    }
    if (length >= 10)
    {
        return 1; // Pseudo trop long
    }
    return 0; // Longueur de pseudo correcte
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
        for (int j = 0; j < server->user_nb; ++j)
            if (strcmp(server->usernames[j], buf[2]) == 0)
                msg = strcat(msg, itoa(j + 1));
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
        int check_result = check_pseudo_length(buf[2]);
        while (check_result != 0)
        {
            if (check_result == -1)
            {
                send_message("Pseudo trop court\n", server->clients[i]);
            }
            else if (check_result == 1)
            {
                send_message("Pseudo trop long\n", server->clients[i]);
            }
            read(server->clients[i].socket, server->clients[i].buffer, 1024);
            buf = string_to_tab(server->clients[i].buffer, ':');
            check_result = check_pseudo_length(buf[2]);
        }
        printf("Taille du pseudo correcte\n");
        if (inscription(server, buf[2]) == -1)
        {
            printf("Pseudo incorrect\n");
            msg = strcat(msg, "Pseudo incorrect\n");
            send_message(msg, server->clients[i]);
            return -1;
        }
        printf("Pseudo correct\n");
        msg = strcat(msg, "Vous etes connecte:");
        for (int j = 0; j < server->user_nb; ++j)
            if (strcmp(server->usernames[j], buf[2]) == 0)
                msg = strcat(msg, itoa(j + 1));
        msg = strcat(msg, "\n");
        send_message(msg, server->clients[i]);
    }
    else
    {
        send_message("Error\n", server->clients[i]);
        return -1;
    }
    server->clients[i].buffer = malloc(sizeof(char) * 1024);
    return 0;
}

int post_message(char *message_content, int thread_id, int id, server_t *server, client_t *client)
{
    printf("%d - %d - %d\n", id, thread_id, server->nb_threads);
    // vérifier si l'utilisateur est inscrit
    if (id <= 0)
    {
        send_message("Vous n'etes pas inscrit\n", *client);
        return -1;
    }

    // trouver le fil
    Thread *thread = NULL;
    for (int i = 0; i < server->nb_threads; ++i)
    {
        if (server->threads[i].id == thread_id)
        {
            thread = &server->threads[i];
            break;
        }
    }

    if (thread == NULL)
    {
        if (server->nb_threads >= MAX_THREADS)
        {
            send_message("Nombre maximal de fils atteint. Impossible de créer un nouveau fil.\n", *client);
            return -1;
        }

        // Créer un nouveau fil
        thread = &server->threads[server->nb_threads];
        thread->id = server->nb_threads;
        thread->name = NULL; // donner un nom au fil
        thread->messages = NULL;

        // Mettre à jour le nombre de fils
        server->nb_threads++;
    }

    // Créer un nouveau message
    Message *new_message = malloc(sizeof(Message));
    new_message->id = thread->id; // ou utiliser un autre moyen pour générer un identifiant unique pour le message
    new_message->author_id = client->id;
    new_message->content = strdup(message_content);
    new_message->next = NULL;

    // Ajouter le message à la fin de la liste de messages du fil
    Message *last_message = thread->messages;
    if (last_message == NULL)
    {
        thread->messages = new_message;
    }
    else
    {
        while (last_message->next != NULL)
        {
            last_message = last_message->next;
        }
        last_message->next = new_message;
    }

    // confirmation au client
    char *msg = malloc(sizeof(char) * 1024);
    msg = strcat(msg, "Votre message a été ajouté\n");
    send_message(msg, *client);
    return 0;
}

int subscribe_request(int thread_id, int id, server_t *server, client_t *client)
{
    printf("%d - %d - %d\n", id, thread_id, server->nb_threads);
    if (id <= 0)
    {
        send_message("Vous n'etes pas inscrit\n", *client);
        return -1;
    }
    // trouver le fil
    Thread *thread = NULL;
    for (int i = 0; i < server->nb_threads; ++i)
    {
        if (server->threads[i].id == thread_id)
        {
            thread = &server->threads[i];
            break;
        }
    }
    if (thread == NULL)
    {
        send_message("\e[0;35mLe fil saisie n'existe pas encore\e[0m", *client);
        return -1;
    }
    // Générer l'adresse de multidiffusion en fonction de numfil
    char multicastAddr[INET_ADDRSTRLEN];
    snprintf(multicastAddr, INET_ADDRSTRLEN, "239.0.0.%d", thread_id);
    // definition du port en fonction de numfil également
    int multi_port = DEFAULT_MULTI_PORT + thread_id;
    char message[BUF_LEN];
    sprintf(message, "Adresse de multidiffusion : %s sur le Port : %d", multicastAddr, multi_port);

    send_message(message, *client);

    return 0;
}

void get_last_n_messages_from_server(int n, int thread_id, server_t *server, client_t *client)
{
    // Trouver le thread
    Thread *thread = NULL;
    int i = 0;
    for (; i < server->nb_threads; ++i)
    {
        if (server->threads[i].id == thread_id)
        {
            thread = &server->threads[i];
            break;
        }
    }

    if (thread == NULL)
    {
        // Pas de thread avec cet ID
        perror("Aucun thread");
        return;
    }

    // Parcourir les messages à l'envers et envoyer les n derniers
    int count = 0;
    Message *message = thread->messages;
    while (message != NULL && count < n)
    {
        char msg[1024];
        sprintf(msg, "Message from %d: %s\n", message->author_id, message->content);
        // send_message(msg, *client);
        write(client->socket, msg, 1024);
        message = message->next;
        count++;
    }
}

int server_activity(server_t *server)
{
    printf("server activity\n");
    // int activity = 0;
    int new_socket = -1;

    if (FD_ISSET(server->socket, &server->readfds))
    {
        if ((new_socket = accept(server->socket, (struct sockaddr *)&server->address, (socklen_t *)&server->addrlen)) < 0)
            return -1;
        char ip_address[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(server->address.sin6_addr), ip_address, INET6_ADDRSTRLEN);
        printf("New connection, socket fd is %d, ip is : %s, port : %d\n", new_socket, ip_address, ntohs(server->address.sin6_port));
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
                server->clients[i].id = server->nb_clients + 1;
                printf("%d, %d\n", server->clients[i].socket, server->nb_clients);
                // if id != 0 -/> connection
                char *msg = malloc(sizeof(char) * 1024);
                read(server->clients[i].socket, msg, 1024);
                printf("%s\n", msg);
                char **buf = string_to_tab(msg, ':');
                printf("%s - %s - %s\n", buf[0], buf[1], buf[2]);
                if (atoi(buf[0]) == 0)
                {
                    printf("Connection\n");
                    if (connection(server, i) != 0)
                    {
                        close(server->clients[i].socket);
                        server->clients[i].socket = -1;
                        server->nb_clients--;
                    }
                }
                else
                {
                    if (strcmp(buf[0], "1") == 0)
                    {
                        post_message(buf[3], atoi(buf[2]), atoi(buf[1]), server, &server->clients[i]);
                    }
                    else if (strcmp(buf[0], "3") == 0)
                    {
                        subscribe_request(atoi(buf[4]), atoi(buf[3]), server, &server->clients[i]);
                    }
                    else if (strcmp(buf[0], "GET_LAST_N_MESSAGES") == 0)
                    {
                        int thread_id = atoi(buf[2]);
                        int n = atoi(buf[1]);
                        get_last_n_messages_from_server(n, thread_id, server, &server->clients[i]);
                    }
                    else
                    {
                        int thread_id = atoi(buf[1]);
                        int n = atoi(buf[2]);
                        get_last_n_messages_from_server(n, thread_id, server, &server->clients[i]);
                    }
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
    if ((server.socket = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
        exit(84);
    int no = 0;
    int r = setsockopt(server.socket, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no));
    if (r < 0)
        fprintf(stderr, "échec de setsockopt() : (%d)\n", errno);
    int yes = 1;
    r = setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (r < 0)
        fprintf(stderr, "échec de setsockopt() : (%d)\n", errno);
    server.address = (struct sockaddr_in6){
        .sin6_family = AF_INET6,
        .sin6_addr.s6_addr = INADDR_ANY,
        .sin6_port = htons(port)};
    server.addrlen = sizeof(server.address);
    if (bind(server.socket, (struct sockaddr *)&server.address, server.addrlen) < 0)
    {
        free(server.usernames);
        exit(84);
    }
    if (listen(server.socket, 3) < 0)
    {
        free(server.usernames);
        exit(84);
    }
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
    {
        free(server.usernames);
        return -1;
    }
    free(server.usernames);
    return 0;
}