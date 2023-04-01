#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "../../headers/socket.h"
#include "../../headers/users.h"
#include "../../headers/func/func_serveur.h"
#include "../../headers/request_definition.h"

#define SIZE_MESS 100
#define MAX_USERS 100
#define MAX_USERNAME_LEN 10

int generate_user_id() {
    static int current_user_id = 0;
    current_user_id++;
    current_user_id &= 0x7FF;
    return current_user_id;
}

void create_new_user(char *username, int user_id, utilisateur liste[], int nb_utilisateurs) {
    utilisateur user;
    user.pseudo = malloc(sizeof(char*)*MAX_USERS);
    strcpy(user.pseudo, username);
    user.id = user_id;

    liste[nb_utilisateurs] = user;
    nb_utilisateurs++;
}

uint16_t recv_header_client(int sock_client, char *buf, uint16_t header_client) {
    int r = recv(sock_client, buf, SIZE_MESS, 0);
    if (r < 0){
        perror("recv");
        close(sock_client);
        exit(1);
    }
    if(r == 0){
        fprintf(stderr, "send du client nul\n");
        close(sock_client);
        exit(1);
    }

    memcpy(&header_client, buf, 2);
    return header_client;
}

int inscription_request(int sock_client, char *buf, utilisateur liste[], int nb_utilisateurs) {
    uint16_t header_serv, id;
    uint8_t codereq_serv;
    char username[MAX_USERNAME_LEN+1];

    memcpy(username, buf+sizeof(uint16_t), MAX_USERNAME_LEN);
    username[MAX_USERNAME_LEN] = '\0';
    printf("PSEUDONYME RECU : %s \n", username);

    id = generate_user_id();

    create_new_user(username, id, liste, nb_utilisateurs);

    codereq_serv = REQ_INSCRIPTION;

    header_serv = htons((id << 5) | (codereq_serv & 0x1F));
                
    memset(buf, 0, SIZE_MESS);
    memcpy(buf, &header_serv, sizeof(header_serv));

    int r = send(sock_client, buf, SIZE_MESS, 0);
    if (r < 0){
        perror("send");
        close(sock_client);
        exit(1);
    }

    return 0;
}

void error_request(int sock_client, uint8_t codereq_client, uint16_t id) {
    uint8_t codereq_serv;
    uint16_t header_serv;
    char buf[SIZE_MESS];
    int r;

    printf("CODEREQ INCONNU : <%d>\n", codereq_client);
    codereq_serv = -1;
    header_serv = htons((id << 5) | (codereq_serv & 0x1F));
    
    memset(buf, 0, SIZE_MESS);
    memcpy(buf, &header_serv, sizeof(uint16_t));
    
    r = send(sock_client, buf, SIZE_MESS, 0);
    if (r < 0){
        perror("send");
        close(sock_client);
        exit(1);
    }
}