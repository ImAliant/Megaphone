#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#include "../headers/socket.h"
#include "../headers/billet.h"
#include "../headers/users.h"
#include "../headers/request.h"
#include "../headers/error.h"
#include "../headers/func/func_serveur.h"
#include "../headers/message.h"

#define SIZE_MESS 200
#define MAX_USERS 100
#define USERNAME_LEN 10
int port;
struct fils *fils;
int nb_utilisateurs = 0;

utilisateur liste[MAX_USERS];

int is_user_registered(uint16_t id) {
    for(int i = 0; i < nb_utilisateurs; i++) {
        if(liste[i].id == id) {
            return 1;
        }
    }
    return 0;
}

void *serve(void *arg) {
    int r;
    uint16_t header, id;
    uint8_t codereq;
    char username[USERNAME_LEN];

    int sock = *(int *)arg;

    char buf[SIZE_MESS*2];
    memset(buf, 0, SIZE_MESS*2);

    recv_send_message(sock, buf, SIZE_MESS*2, RECV);

    memcpy(&header, buf, sizeof(uint16_t));
    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;

    // ENVOIE ENTETE ERREUR SI ID DIFFERENT DE 0 LORSQUE CODEREQ 1
    if (codereq == REQ_INSCRIPTION && id != 0) {
        error_request(sock, codereq, id, ERR_NON_ZERO_ID_WITH_CODE_REQ_ONE);
        return NULL;
    }
    // ENVOIE ENTETE ERREUR SI ID N'EXISTE PAS POUR LES AUTRES REQUETES
    if (codereq != REQ_INSCRIPTION && !is_user_registered(id)) {
        error_request(sock, codereq, id, ERR_ID_DOES_NOT_EXIST);
        return NULL;
    }

    switch(codereq) {
        case REQ_INSCRIPTION:
            r = inscription_request(sock, buf, liste, nb_utilisateurs);
            if (r==0) nb_utilisateurs++;
            break;
        case REQ_POST_BILLET:
            for(int i = 0; i < nb_utilisateurs; i++) {
                if(liste[i].id == id) {
                    strcpy(username, liste[i].pseudo);
                    break;
                }
            }

            r = post_billet_request(sock, buf, fils, username);
            break;
        case REQ_GET_BILLET:
            r = get_billets_request(sock, buf, fils);
            break;
        case REQ_SUBSCRIBE:
            r = subscribe_request(sock,buf,port);
            break;
        case REQ_ADD_FILE:
            /*r = add_file_request();
            break;*/
        case REQ_DW_FILE:
            /*r = download_file_request();*/
            break;
        default:
            error_request(sock, codereq, id, ERR_CODEREQ_UNKNOWN);
            break;
    }

    close(sock);

    return NULL;
}

void loop(int sock) {
    while(1) {
        struct sockaddr_in6 addrclient;
        socklen_t size=sizeof(addrclient);
    
        //*** on crée la varaiable sur le tas ***
        int *sock_client = malloc(sizeof(int));

        //*** le serveur accepte une connexion et initialise la socket de communication avec le client ***
        *sock_client = accept_connexion(sock, addrclient, size);

        if (sock_client >= 0) {
            pthread_t thread;
            //*** le serveur cree un thread et passe un pointeur sur socket client à la fonction serve ***
            if (pthread_create(&thread, NULL, serve, sock_client) == -1) {
	            perror("pthread_create");
	            continue;
            }  
            //*** affichage de l'adresse du client ***
            char nom_dst[INET6_ADDRSTRLEN];
            printf("CONNEXION CLIENT : %s %d\n", inet_ntop(AF_INET6,&addrclient.sin6_addr,nom_dst,sizeof(nom_dst)), htons(addrclient.sin6_port));
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2){
        fprintf(stderr, "usage : ./serveur <PORT>\n");
        exit(1);
    }

    fils = malloc(sizeof(struct fils));
    fils->nb_fil = 0;

    int sock;
    struct sockaddr_in6 address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    
    port = atoi(argv[1]);
    addresse_destinataire(port, &address_sock);

    sock = creation_socket();

    desac_option_only_ipv6(sock);

    parallel_use_port(sock);

    bind_port(sock, &address_sock);

    listen_port(sock);

    loop(sock);
  
    close(sock);

    free(fils);

    return 0;
}