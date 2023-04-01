#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "../headers/socket.h"
#include "../headers/users.h"
#include "../headers/request_definition.h"
#include "../headers/func/func_serveur.h"

#define SIZE_MESS 100
#define MAX_USERS 100

int nb_utilisateurs = 0;

utilisateur liste[MAX_USERS];

void create_new_user(char *username, int user_id) {
    utilisateur user;
    user.pseudo = malloc(sizeof(char*)*MAX_USERS);
    strcpy(user.pseudo, username);
    user.id = user_id;

    liste[nb_utilisateurs] = user;
    nb_utilisateurs++;
}

void loop_communication_server_client(int sock) {
    while(1) {
        uint16_t header_client, header_serv, id;
        uint8_t codereq_client, codereq_serv;
        struct sockaddr_in6 adr;
        socklen_t lg = sizeof(struct sockaddr_in6);

        int sock_client = accept_connexion(sock, adr, &lg);

        char buf[SIZE_MESS];
        memset(buf, 0, SIZE_MESS);

        header_client = recv_header_client(sock_client, buf, header_client);
        codereq_client = ntohs(header_client) & 0x1F;
        id = ntohs(header_client) >> 5;

        switch(codereq_client) 
        {
            case REQ_INSCRIPTION:
                inscription_request(sock_client, buf);
                break;
            case REQ_POST_BILLET:
            case REQ_GET_BILLET:
            case REQ_SUBSCRIBE:
            case REQ_ADD_FILE:
            case REQ_DW_FILE:
                break;
            default:
                error_request(sock_client, codereq_client, id);
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2){
        fprintf(stderr, "usage : ./serveur <PORT>\n");
        exit(1);
    }

    int port, sock, r;
    struct sockaddr_in6 address_sock;
    
    port = atoi(argv[1]);
    addresse_destinataire(port, &address_sock);

    sock = creation_socket();

    desac_option_only_ipv6(sock);

    parallel_use_port(sock);

    bind_port(sock, &address_sock);

    listen_port(sock);

    loop_communication_server_client(sock);

    close(sock);

    return 0;
}