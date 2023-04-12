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
#include "include/color.h"


void gestion_inscription(int sock, int id, int codereq) {
    // Entête du message client
    uint16_t header = htons((id << 5) | (codereq & 0x1F));
    printf("HEADER : %hu\n",header);
    char ret[10 + 1]; // +1 pour le caractère de fin de chaîne
    printf("Veuillez entrer un pseudo :\n");
    scanf("%s", ret);

    // Complétion par des # si la longueur est inférieure à 10 caractères
    int len = strlen(ret);
    if (len < PSEUDO_LEN) {
        for (int i = len; i < PSEUDO_LEN; i++) {
            ret[i] = '#';
        }
        ret[PSEUDO_LEN] = '\0';
    }

    printf("val de pseudo : %s\n", ret);

    int snd = send(sock, &header, sizeof(header), 0);  // envoi de l'entête
    if(snd < 0) {
        perror("Erreur d'envoi ... \n");
        exit(EXIT_FAILURE);
    }
    snd = send(sock, ret, PSEUDO_LEN, 0); //envoi du pseudo
    if(snd < 0) {
        perror("Erreur d'envoi ... \n");
        exit(EXIT_FAILURE);
    }
}

void reception_inscription(int sock) {
    // Réception de la réponse du serveur
    uint16_t response_header[2];
    uint8_t code_req, num_fil;
    uint16_t response_id, nb;

    if (recv(sock, response_header, sizeof(response_header), 0) == -1) {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }

    code_req = ntohs(response_header[0] >> 5 & 0x1F);
    response_id = ntohs(response_header[1] & 0x7FF); // Récupération des 11 bits de poids forts
    printf("Received message :\nCode de requête -> %hu\nID client(garder l'id) -> %hu\n",code_req,response_id);
    
    if (recv(sock, &num_fil, sizeof(num_fil), 0) == -1) {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }
    if (recv(sock, &nb, sizeof(nb), 0) == -1) {
        perror("Erreur lors de la réception de la réponse du serveur");
        exit(EXIT_FAILURE);
    }
    printf("Numéro de fil -> %u\nNB -> %u\n", num_fil, nb);
}

int not_req(u_int8_t req){
    return req != 1 || req != 2 || req != 3 || req != 4 || req != 5 || req != 6;
}

void print_help() {
    printf("Les requetes suivantes sont de la forme :\n1 - ...\n");
}

void gestion_req(int sock,int client_id,int codereq) {
    printf("Non gérer...\n");
}
void reception_msg(int sock) {
    printf("Non gérer...\n");
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
    uint8_t codereq;
    printf("Connected to server\nBienvenue sur le protocole Mégaphone\nVeuillez entrer votre ID(0 si première connexion):\n");
    scanf("%hu", &client_id);
    if(client_id == 0) {
        codereq = 1;
        gestion_inscription(sock, client_id, codereq);
        reception_inscription(sock);
    } 

    while (1){
        printf("Quel est l'objet de votre requete ?\n");
        scanf("%hhu", &codereq);
        //printf("codereq : %hu\n",codereq);
        if(codereq == 0) {
            break;
        }
        if(not_req(codereq)) {
            print_help();
        }
        gestion_req(sock,client_id, codereq);
        reception_msg(sock);
    }
    
    close(sock);
    return 0;
}
