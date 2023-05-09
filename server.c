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

#include <ctype.h>

void print_buffer(const char *buffer, int size)
{
    printf("Buffer content (size = %d):\n", size);
    for (int i = 0; i < size; i++)
    {
        if (isprint(buffer[i]))
        {
            printf("%c ", buffer[i]);
        }
        else
        {
            printf("%02X ", (unsigned char)buffer[i]);
        }
    }
    printf("\n");
}
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

typedef struct Billet
{
    uint32_t user_id;
    char message[MESSAGE_LEN];
    struct Billet *next;
} Billet;

typedef struct Fil
{
    char num_fichier[PSEUDO_LEN];
    Billet *billet;
    struct Fil *next;
} Fil;

Fil *fils = NULL;

int utilisateur_existe(uint32_t id, const char *pseudo)
{
    int pseudo_len = strlen(pseudo);
    for (int i = 0; i < liste_utilisateurs.nombre; i++)
    {
        if (liste_utilisateurs.utilisateurs[i].id == id &&
            strlen(liste_utilisateurs.utilisateurs[i].pseudo) == pseudo_len &&        
            strncmp(liste_utilisateurs.utilisateurs[i].pseudo, pseudo, pseudo_len) == 0)
        {
            return 1;
        }
        printf("Vérification de l'utilisateur: ID: %u, Pseudo: %s\n", liste_utilisateurs.utilisateurs[i].id, liste_utilisateurs.utilisateurs[i].pseudo);
    }
    return 0;
}

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

void ajouter_billet(uint32_t user_id, const char *num_fichier, const char *message_billet)
{
    Fil *fil = NULL;

    // Rechercher le fil existant ou en créer un nouveau
    for (Fil *temp = fils; temp != NULL; temp = temp->next)
    {
        if (strcmp(temp->num_fichier, num_fichier) == 0)
        {
            fil = temp;
            break;
        }
    }

    if (fil == NULL)
    {
        fil = (Fil *)malloc(sizeof(Fil));
        strncpy(fil->num_fichier, num_fichier, PSEUDO_LEN);
        fil->billet = NULL;
        fil->next = fils;
        fils = fil;
    }

    // Créer un nouveau billet
    Billet *nouveau_billet = (Billet *)malloc(sizeof(Billet));
    nouveau_billet->user_id = user_id;
    strncpy(nouveau_billet->message, message_billet, MESSAGE_LEN);
    nouveau_billet->next = NULL;

    // Ajouter le billet à la fin du fil
    if (fil->billet == NULL)
    {
        fil->billet = nouveau_billet;
    }
    else
    {
        Billet *temp = fil->billet;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = nouveau_billet;
    }

    printf("Billet ajouté avec succès par l'utilisateur %u au fil %s\n", user_id, num_fichier);
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
    print_buffer(buffer, received);
    if (buffer[0] == CODEREQ)
    {
        if (buffer[1] == INSCRIPTION)
        {
            char pseudo[PSEUDO_LEN + 1];
            memcpy(pseudo, &buffer[2], PSEUDO_LEN);
            pseudo[PSEUDO_LEN] = '\0';
            int actual_pseudo_len = strnlen(pseudo, PSEUDO_LEN);
            memset(pseudo + actual_pseudo_len, 0, PSEUDO_LEN - actual_pseudo_len);

            printf("Demande d'inscription pour le pseudo: %s\n", pseudo);

            uint32_t user_id = inscrire_utilisateur(pseudo);

            uint32_t user_id_net = htonl(user_id);
            int sent = send(client_sock, &user_id_net, sizeof(user_id_net), 0);
            if (sent < 0)
            {
                perror("send");
            }
            else
            {
                printf("Utilisateur inscrit avec succès. Pseudo: %s, ID: %u\n", pseudo, user_id);
            }
        }
        else if (buffer[1] == POST_BILLET)
        {
            uint32_t user_id = ntohl(*((uint32_t *)&buffer[2]));
            char num_fichier[PSEUDO_LEN + 1];
            memcpy(num_fichier, &buffer[4], PSEUDO_LEN);
            num_fichier[PSEUDO_LEN] = '\0';

            printf("Demande d'ajout de billet par l'utilisateur ID: %hu pour le fil: %s\n", user_id, num_fichier);

            ajouter_billet(user_id, num_fichier, "MESSAGE_BILLET");

            char response[] = "Billet ajouté avec succès";
            int sent = send(client_sock, response, sizeof(response), 0);
            if (sent < 0)
            {
                perror("send");
            }
        }
        else if (buffer[1] == LIST_BILLET)
        {
            uint32_t user_id = ntohl(*((uint32_t *)&buffer[2]));
            printf("Demande de lister les billets par l'utilisateur ID: %hu\n", user_id);

            // Recevoir le nombre de billets à afficher
            uint8_t n;
            int rcv = recv(client_sock, &n, sizeof(n), 0);
            if (rcv < 0)
            {
                perror("recv");
                exit(EXIT_FAILURE);
            }

            Billet *current_billet = NULL;
            Fil *current_fil = fils;

            while (current_fil != NULL)
            {
                current_billet = current_fil->billet;
                while (current_billet != NULL)
                {
                    char response[MESSAGE_LEN];
                    snprintf(response, sizeof(response), "Billet de l'utilisateur %u : %s", current_billet->user_id, current_billet->message);
                    int sent = send(client_sock, response, sizeof(response), 0);
                    if (sent < 0)
                    {
                        perror("send");
                    }
                    current_billet = current_billet->next;
                }
                current_fil = current_fil->next;
            }
        }
        else if (buffer[1] == USER_EXISTS)
        {
            uint32_t user_id = ntohl(*((uint32_t *)&buffer[2]));
            char pseudo[PSEUDO_LEN + 1];
            memcpy(pseudo, &buffer[4], PSEUDO_LEN);
            pseudo[PSEUDO_LEN] = '\0';

            printf("Vérification de l'existence de l'utilisateur ID: %hu, Pseudo: %s\n", user_id, pseudo);

            if (utilisateur_existe(user_id, pseudo))
            {
                uint8_t response = 1; // Utilisez 1 pour indiquer que l'utilisateur existe
                print_buffer((char *)&response, sizeof(response));
                int sent = send(client_sock, &response, sizeof(response), 0);
                if (sent < 0)
                {
                    perror("send");
                }
            }
            else
            {
                uint8_t response = 0; // Utilisez 0 pour indiquer que l'utilisateur n'existe pas
                print_buffer((char *)&response, sizeof(response));
                int sent = send(client_sock, &response, sizeof(response), 0);

                if (sent < 0)
                {
                    perror("send");
                }
            }
        }

        else
        {
            printf("Requête non prise en charge\n");
        }
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