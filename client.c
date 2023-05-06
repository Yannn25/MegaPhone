#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
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

char *gestion_pseudo()
{
    printf("Veuillez entrer un pseudo :\n");
    char *ret = malloc(PSEUDO_LEN);
    while (1)
    {
        // memset(ret, 0, PSEUDO_LEN);
        scanf("%s", ret);
        if (strlen(ret) < 10 && strcmp(ret, "") != 0)
            break;
        else
            printf("Le pseudo choisi est trop long recommencez :\n");
    }
    for (int i = strlen(ret); i < PSEUDO_LEN; i++)
    {
        ret[i] = '#';
    }
    printf("val de pseudo : %s\n", ret);
    return ret;
}

int creer_socket()
{
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("échec création socket...\n");
        exit(EXIT_FAILURE);
    }
    return sock;
}

int utilisateur_existe(int socket, uint16_t id_client, const char *pseudo)
{
    char buffer[3 + PSEUDO_LEN];
    memset(buffer, 0, sizeof(buffer));

    buffer[0] = CODEREQ;
    buffer[1] = USER_EXISTS;
    *((uint16_t *)&buffer[2]) = htons(id_client);
    memcpy(&buffer[4], pseudo, PSEUDO_LEN - 1);
    print_buffer(buffer, sizeof(buffer));
    if (send(socket, buffer, sizeof(buffer), 0) == -1)
    {
        perror("Erreur d'envoi du nom d'utilisateur et de l'ID client");
        exit(EXIT_FAILURE);
    }

    memset(buffer, 0, sizeof(buffer));

    if (recv(socket, buffer, sizeof(buffer), 0) == -1)
    {
        perror("Erreur de réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }
    print_buffer(buffer, sizeof(buffer));
    if (strncmp(buffer, "YES", 3) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void se_connecter_au_serveur(int sock)
{
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT);
    inet_pton(AF_INET6, ADDR, &addr.sin6_addr);

    int c = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (c < 0)
    {
        perror("failure on connection...\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\nVeuillez envoyez votre requete :\n");
}

void inscrire_utilisateur(int sock)
{
    char *pseudo = gestion_pseudo();

    // le client se connecte au serveur et envoie une demande d’inscription en donnant le pseudo de l’utilisateur.
    char bufenv[2 + PSEUDO_LEN];
    bufenv[0] = CODEREQ;
    bufenv[1] = INSCRIPTION;
    memcpy(&bufenv[2], pseudo, PSEUDO_LEN);
    print_buffer(bufenv, sizeof(bufenv));
    int env = send(sock, bufenv, sizeof(bufenv), 0);
    if (env < 0)
    {
        perror("Erreur d'envoi...\n");
        exit(EXIT_FAILURE);
    }
    free(pseudo);
}

void poster_billet(int sock, uint16_t id, const char *num_fichier, const char *message_billet)
{
    char bufenv[4 + PSEUDO_LEN];
    bufenv[0] = CODEREQ;
    bufenv[1] = POST_BILLET;
    *((uint16_t *)&bufenv[2]) = htons(id);
    memcpy(&bufenv[4], num_fichier, PSEUDO_LEN);

    int env = send(sock, bufenv, sizeof(bufenv), 0);
    if (env < 0)
    {
        perror("Erreur d'envoi...\n");
        exit(EXIT_FAILURE);
    }

    // Réception de la réponse du serveur
    char bufrec[256];
    memset(bufrec, 0, sizeof(bufrec));
    int rec = recv(sock, bufrec, sizeof(bufrec) - 1, 0);
    if (rec < 0)
    {
        perror("Erreur de réception...\n");
        exit(EXIT_FAILURE);
    }
    bufrec[rec] = '\0';
    printf("Réponse serveur (ajout du billet): %s\n", bufrec);

    close(sock);
}

uint16_t reception_inscription(int sock)
{
    // Réception de la réponse du serveur
    uint16_t response_header[2];
    uint8_t code_req, num_fil;
    uint16_t response_id, nb;

    if (recv(sock, response_header, sizeof(response_header), 0) == -1)
    {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }

    code_req = ntohs(response_header[0] >> 5 & 0x1F);
    response_id = ntohs(response_header[1]);
    printf("Received message :\nCode de requête -> %hu\nID client(garder l'id) -> %hu\n", code_req, response_id);

    if (recv(sock, &num_fil, sizeof(num_fil), 0) == -1)
    {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }

    if (recv(sock, &nb, sizeof(nb), 0) == -1)
    {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }
    print_buffer((char *)response_header, sizeof(response_header));
    printf("Numéro de fil -> %u\nNB -> %u\n", num_fil, nb);

    return response_id;
}

void lister_derniers_billets(int sock, uint16_t id, uint8_t n)
{
    char bufenv[4];
    bufenv[0] = CODEREQ;
    bufenv[1] = LIST_BILLET;
    *((uint16_t *)&bufenv[2]) = htons(id);

    int env = send(sock, bufenv, sizeof(bufenv), 0);
    if (env < 0)
    {
        perror("Erreur d'envoi...\n");
        exit(EXIT_FAILURE);
    }

    // Envoyer le nombre de billets à afficher
    int sent = send(sock, &n, sizeof(n), 0);
    if (sent < 0)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }

    // Réception de la réponse du serveur
    char bufrec[MESSAGE_LEN];
    for (int i = 0; i < n; i++)
    {
        memset(bufrec, 0, sizeof(bufrec));
        int rec = recv(sock, bufrec, sizeof(bufrec) - 1, 0);
        if (rec < 0)
        {
            perror("Erreur de réception...\n");
            exit(EXIT_FAILURE);
        }
        bufrec[rec] = '\0';
        printf("Billet %d: %s\n", i + 1, bufrec);
    }
}

void menu(int sock, uint16_t id_client)
{
    char choix;
    char num_fichier[10], message_billet[256];

    while (1)
    {
        printf("\nMenu:\n1. Poster un billet (p)\n2. Lister les n derniers billets (l)\n3. Abonnement à un fil (s)\n4. Quitter (q)\n");
        printf("Entrez votre choix: ");
        scanf(" %c", &choix);

        switch (choix)
        {
        case 'p':
        {
            printf("Entrez le numéro du fichier: ");
            scanf("%s", num_fichier);
            printf("Entrez le message du billet: ");
            scanf(" %[^\n]s", message_billet);

            // Poster un billet
            int sock_poster = creer_socket();
            se_connecter_au_serveur(sock_poster);
            poster_billet(sock_poster, id_client, num_fichier, message_billet);
        }

        break;

        case 'l':
            // Lister les n derniers billets
            {
                uint8_t n;
                printf("Entrez le nombre de billets à afficher: ");
                scanf("%hhu", &n);
                int sock_lister = creer_socket();
                se_connecter_au_serveur(sock_lister);
                lister_derniers_billets(sock_lister, id_client, n);
                close(sock_lister);
            }
            break;

        case 's':
            // Implémenter la fonction pour s'abonner à un fil
            break;

        case 'q':
            printf("Au revoir!\n");
            exit(0);
            break;

        default:
            printf("Option invalide. Réessayez.\n");
            break;
        }
    }
}

int main(int argc, char **argv)
{
    char choix_inscription;
    uint16_t id_client;

    printf("Êtes-vous déjà inscrit ? (O/N) : ");
    scanf(" %c", &choix_inscription);

    if (choix_inscription == 'N' || choix_inscription == 'n')
    {
        // Inscription d'un utilisateur
        int sock_inscription = creer_socket();
        se_connecter_au_serveur(sock_inscription);
        inscrire_utilisateur(sock_inscription);
        id_client = reception_inscription(sock_inscription);
        printf("Votre ID client est : %d\n", id_client);
        close(sock_inscription); // Fermez la socket après avoir reçu l'ID client
    }
    if (choix_inscription == 'O' || choix_inscription == 'o')
    {
        char pseudo[20];
        printf("Entrez votre pseudo : ");
        scanf("%s", pseudo);
        printf("Entrez votre ID client : ");
        scanf("%hu", &id_client);

        // Vérifiez si l'utilisateur existe sur le serveur
        int sock_verification = creer_socket();
        se_connecter_au_serveur(sock_verification);
        if (utilisateur_existe(sock_verification, id_client, pseudo))
        {
            close(sock_verification); // Fermez la socket de vérification avant d'en créer une nouvelle
            // Affichage du menu et gestion des actions
            int sock_menu = creer_socket();
            se_connecter_au_serveur(sock_menu);
            menu(sock_menu, id_client);
        }

        else
        {
            printf("L'utilisateur n'existe pas. Veuillez vous inscrire d'abord.\n");
        }
        close(sock_verification);
    }
    else
    {
        return 0;
    }
}