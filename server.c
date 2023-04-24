#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "include/global.h"

typedef struct
{
    uint32_t id;
    char pseudo[PSEUDO_LEN];
} Utilisateur;

typedef struct
{
    Utilisateur *utilisateurs;
    int nombre;
} ListeUtilisateurs;

ListeUtilisateurs liste_utilisateurs = {NULL, 0};
uint32_t dernier_id = 0;

uint32_t inscrire_utilisateur(const char *pseudo)
{
    Utilisateur utilisateur;
    utilisateur.id = ++dernier_id;
    strncpy(utilisateur.pseudo, pseudo, PSEUDO_LEN);

    Utilisateur *new_utilisateurs = realloc(liste_utilisateurs.utilisateurs,
                                            sizeof(Utilisateur) * (liste_utilisateurs.nombre + 1));
    if (new_utilisateurs == NULL)
    {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    liste_utilisateurs.utilisateurs = new_utilisateurs;
    liste_utilisateurs.utilisateurs[liste_utilisateurs.nombre] = utilisateur;
    liste_utilisateurs.nombre++;

    return utilisateur.id;
}

void *gerer_client(void *arg)
{
    int client_sock = *(int *)arg;
    free(arg);
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int received = recv(client_sock, buffer, sizeof(buffer), 0);
    if (received < 0)
    {
        perror("recv");
        close(client_sock);
        return NULL;
    }

    if (buffer[0] == CODEREQ)
    {
        char pseudo[PSEUDO_LEN + 1];
        memcpy(pseudo, &buffer[2], PSEUDO_LEN);
        pseudo[PSEUDO_LEN] = '\0';
        printf("Demande d'inscription avec le pseudo : %s\n", pseudo);

        uint32_t user_id = inscrire_utilisateur(pseudo);
        printf("Utilisateur inscrit avec l'ID : %u\n", user_id);

        char response[9];
        snprintf(response, sizeof(response), "ID%08u", user_id);

        int sent = send(client_sock, response, sizeof(response), 0);
        if (sent < 0)
        {
            perror("send");
        }
    }
    else
    {
        printf("Requête non prise en charge\n");
    }

    close(client_sock);
    return NULL;
}

int main(int argc, char **argv)
{
    liste_utilisateurs.nombre = 0;
    liste_utilisateurs.utilisateurs = malloc(sizeof(Utilisateur) * liste_utilisateurs.nombre);

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("échec création socket...\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT);
    addr.sin6_addr = in6addr_any;

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(sock, 10) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1)
    {
        struct sockaddr_in6 client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0)
        {
            perror("accept");
            continue;
        }

        printf("Connexion acceptée\n");

        int *client_sock_ptr = malloc(sizeof(int));
        if (client_sock_ptr == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        *client_sock_ptr = client_sock;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, gerer_client, client_sock_ptr) != 0)
        {
            perror("pthread_create");
            close(client_sock);
            free(client_sock_ptr);
        }
        else
        {
            pthread_detach(thread_id);
        }
    }

    close(sock);
    free(liste_utilisateurs.utilisateurs);

    return 0;
}
