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


void gestion_inscription(int sock) {
    // Entête du message client
    uint16_t header = htons((ID << 5) | (CODEREQ & 0x1F));
    char *ret = malloc(PSEUDO_LEN);
    printf("Veuillez entrer un pseudo :\n");
    while(1) {
        scanf("%s", ret);
        if(strlen(ret) <= PSEUDO_LEN)
            break;
        else {
            memset(ret, 0, strlen(ret));
            printf("Le pseudo choisi est trop long recommencez:\n");
        }   
    }
    for (int i = strlen(ret); i < PSEUDO_LEN; i++) {
        ret[i] = '#';
    }
    printf("val de pseudo : %s\n", ret);
    int snd = send(sock, &header, sizeof(header), 0);  // envoi de l'entête
    if(snd < 0) {
        perror("Erreur d'envoi ... \n");
        exit(EXIT_FAILURE);
    }
    snd = send(sock,ret,PSEUDO_LEN,0);
     if(snd < 0) {
        perror("Erreur d'envoi ... \n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    //création de notre socket
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("échec création socket...\n");
        exit(EXIT_FAILURE);
    }

    //préparation de notre adresse
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT);
    inet_pton(AF_INET6, ADDR, &addr.sin6_addr);//addr = "lulu" sur le port 7777

    //liaison socket et adresse
    int c = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
    if(c < 0) {
        perror("failure on connection...\n");
        exit(EXIT_FAILURE);
    }
    //accueil utilisateur
    uint16_t client_id;
    printf("Connected to server\nBienvenue sur le protocole Mégaphone\nVeuillez entrer votre ID(0 si première connexion):\n");
    scanf("%hu", &client_id);
    if(client_id == 0)
        gestion_inscription(sock);
    else {
        printf("Requete non gerer pour le moment ...\n");
        close(sock);
        return 0;
    }
        

    // Réception de la réponse du serveur
    uint16_t response_header[2];
    uint8_t code_req, num_fil;
    uint16_t response_id, nb;

    if (recv(sock, response_header, sizeof(response_header), 0) == -1) {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }

    code_req = ntohs(response_header[0]);
    response_id = ntohs(response_header[1]);
    printf("Received message :\nCode de requête -> %u\nID client(garder l'id) -> %u\n",code_req,response_id);
    
    if (recv(sock, &num_fil, sizeof(num_fil), 0) == -1) {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }
    if (recv(sock, &nb, sizeof(nb), 0) == -1) {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }
    printf("Numéro de fil -> %u\nNB -> %u\n", num_fil, nb);

    close(sock);
    return 0;
}
