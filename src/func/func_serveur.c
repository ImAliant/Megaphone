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

#define SIZE_MESS 100
#define MAX_USERS 100
#define MAX_USERNAME_LEN 10

#define REQ_INSCRIPTION 1
#define REQ_POST_BILLET 2
#define REQ_GET_BILLET 3
#define REQ_SUBSCRIBE 4
#define REQ_ADD_FILE 5
#define REQ_DW_FILE 6

int generate_user_id() {
    static int current_user_id = 0;
    current_user_id++;
    current_user_id &= 0x7FF;
    return current_user_id;
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

void inscription_request(int sock_client, char *buf) {
    uint16_t header_serv, id;
    uint8_t codereq_serv;
    char username[MAX_USERNAME_LEN+1];

    memcpy(username, buf+sizeof(uint16_t), MAX_USERNAME_LEN);
    username[MAX_USERNAME_LEN] = '\0';
    printf("PSEUDONYME RECU : %s \n", username);

    id = generate_user_id();

    create_new_user(username, id);

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