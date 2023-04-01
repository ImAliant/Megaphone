#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#include "../../headers/socket.h"
#include "../../headers/func/func_client.h"
#include "../../headers/request_definition.h"

#define SIZE_MESS 100
#define MAX_USERNAME_LEN 10

void error_request(uint8_t codereq_serv) {
    if (codereq_serv < 0 && codereq_serv > 6) {
        printf("ERREUR : REQUETE INVALIDE <%d> \n", codereq_serv);
        exit(1);
    }
}

void remove_special_chars(char *str) {
    int i, j;
    for (i = 0, j = 0; str[i]; i++) {
        if (!isspace(str[i]) && isprint(str[i])) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

void completion_pseudo(char *username) {
    int len = strlen(username);
    if (len < MAX_USERNAME_LEN) {
        memset(username+len, '#', MAX_USERNAME_LEN-len);
        username[MAX_USERNAME_LEN] = '\0';
        remove_special_chars(username);
    }
}

void demande_pseudo(char *username) {
    printf("Saisir votre pseudo : ");
    memset(username, 0, MAX_USERNAME_LEN+1);
    fgets(username, MAX_USERNAME_LEN, stdin);
    completion_pseudo(username);
    printf("PSEUDO : %s \n", username);
}

uint16_t create_header(uint8_t codereq_client) {
    uint16_t id = 0;
    uint16_t header_client = htons((id << 5) | (codereq_client & 0x1F));

    return header_client;
}

void header_username_buffer(char *buf, uint16_t header_client, char *username) {
    memcpy(buf, &header_client, sizeof(header_client));
    memcpy(buf+sizeof(header_client), username, strlen(username));
    buf[sizeof(header_client)+strlen(username)] = '\0';
}

int connexion_server(char *hostname, char *port) {
    int sock;
    struct sockaddr_in6* server_addr;
    int adrlen;

    switch (get_server_addr(hostname, port, &sock, &server_addr, &adrlen)) 
    {
    case 0: printf("adresse creee !\n"); break;
    case -1:
        fprintf(stderr, "Erreur: hote non trouve.\n"); 
    case -2:
        fprintf(stderr, "Erreur: echec de creation de la socket.\n");
    exit(1);
    }

    return sock;
}

void inscription_request(char *hostname, char *port) {
    int sock;
    uint16_t header_client, header_serv, id_serv;
    uint8_t codereq_serv;
    char username[MAX_USERNAME_LEN+1];
    char buf[SIZE_MESS];
    
    codereq_serv = REQ_INSCRIPTION;
    
    header_client = create_header(codereq_serv);

    demande_pseudo(username);

    header_username_buffer(buf, header_client, username);

    sock = connexion_server(hostname, port);

    // ENVOI ENTETE + PSEUDO
    int ecrit = send(sock, buf, sizeof(header_client)+strlen(username), 0);
    if(ecrit <= 0){
        perror("erreur ecriture");
        exit(3);
    }

    // RECEPTION ENTETE + ID
    memset(buf, 0, SIZE_MESS);
    int r = recv(sock, buf, SIZE_MESS, 0);
    if(r <= 0){
        perror("erreur lecture");
        exit(4);
    }

    // DECODAGE ENTETE + ID
    memcpy(&header_serv, buf, sizeof(header_serv));
    id_serv = ntohs(header_serv) >> 5;
    codereq_serv = ntohs(header_serv) & 0x1F;
    
    error_request(codereq_serv);

    printf("VOICI VOTRE ID : %d \n", id_serv);

    close(sock);
}