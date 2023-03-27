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

uint16_t PSEUDO_LEN = 10;

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
    printf("Connected to server\nVeuillez envoyez votre requete :\n");
    // Entête du message client
    char header[2] = {CODEREQ, ID}; // CODEREQ et ID sont des entiers de 1 octet
    uint16_t header_length = sizeof(header);
    char *pseudo = gestion_pseudo();
    // Concaténer l'entête et le pseudo
    char *message = malloc(header_length + PSEUDO_LEN + 1);
    memcpy(message, header, header_length);
    char *message_ptr = message;  // Déclaration du pointeur pour l'entête
    message_ptr += header_length;  // On déplace le pointeur à la fin de l'entête
    memcpy(message_ptr, pseudo, PSEUDO_LEN);
    //memset(message + header_length + PSEUDO_LEN, '\0', 1);
    uint16_t msg_length = strlen(message);  // La longueur de la chaîne de caractères
    uint16_t big_endian_length = htons(msg_length);  // La longueur en format big-endian
    char encoded_msg[ENT_SIZE];  
    memcpy(encoded_msg + sizeof(big_endian_length), message, msg_length);  // Copie la chaîne de caractères

    printf("Message client : %s\n", message);
    int snd = send(sock, message, sizeof(message), 0);
    if(snd < 0) {
        perror("Erreur d'envoi ... \n");
        exit(EXIT_FAILURE);
    }

    //Lecture du message recue
    char *bufrec = malloc(sizeof(char) * 12);//a remplacer en bits et non char
    memset(&bufrec, 0, sizeof(bufrec));
    int rec = recv(sock,bufrec,sizeof(bufrec),0);
    if(rec < 0) {
        perror("Erreur de réception...\n");
        exit(EXIT_FAILURE);
    }
    bufrec[rec] = '\0';
    printf("Received message (garder l'id): %s\n", bufrec);

    close(sock);
    return 0;
}