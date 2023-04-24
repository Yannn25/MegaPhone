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

char * gestion_pseudo() {
    char *ret = malloc(PSEUDO_LEN);
    while(1) {
        //memset(ret, 0, PSEUDO_LEN);
        scanf("%s", ret);
        if(strlen(ret) <= 10 || strcmp(ret,"") != 0)
            break;
        else
            printf("Le pseudo choisi est trop long recommencez :\n");
    }
    for (int i = strlen(ret); i < PSEUDO_LEN; i++) {
        ret[i] = '#';
    }
    printf("val de pseudo : %s\n", ret);
    return ret;
}

int creer_socket() {
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("échec création socket...\n");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void se_connecter_au_serveur(int sock) {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT);
    inet_pton(AF_INET6, ADDR, &addr.sin6_addr);

    int c = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (c < 0) {
        perror("failure on connection...\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\nVeuillez envoyez votre requete :\n");
}

void inscrire_utilisateur(int sock) {
    char *pseudo = gestion_pseudo();
    
    //le client se connecte au serveur et envoie une demande d’inscription en donnant le pseudo de l’utilisateur.
    char bufenv[2 + PSEUDO_LEN];
    bufenv[0] = CODEREQ;
    bufenv[1] = INSCRIPTION;
    memcpy(&bufenv[2], pseudo, PSEUDO_LEN);
    int env = send(sock, bufenv, sizeof(bufenv), 0);
    if (env < 0) {
        perror("Erreur d'envoi...\n");
        exit(EXIT_FAILURE);
    }
    free(pseudo);
}


void poster_billet(int sock, const char *id, const char *num_fichier, const char *message_billet) {

    char bufrec[256];
    memset(bufrec, 0, sizeof(bufrec));
    int rec = recv(sock, bufrec, sizeof(bufrec) - 1, 0);
    if (rec < 0) {
        perror("Erreur de réception...\n");
        exit(EXIT_FAILURE);
    }
    bufrec[rec] = '\0';
    printf("Réponse serveur (ajout du billet): %s\n", bufrec);

    close(sock);
}

int main(int argc, char **argv) {
    // Inscription d'un utilisateur
    int sock_inscription = creer_socket();
    se_connecter_au_serveur(sock_inscription);
    inscrire_utilisateur(sock_inscription);

    // Poster un billet
    int sock_poster = creer_socket();
    se_connecter_au_serveur(sock_poster);
    poster_billet(sock_poster, "ID_UTILISATEUR", "NUM_FIL", "MESSAGE_BILLET");

    return 0;
}
