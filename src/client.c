#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#include "../headers/socket.h"
#include "../headers/request.h"
#include "../headers/func/func_client.h"

#define SIZE_MESS 200
#define MAX_USERNAME_LEN 10
#define SIZE_STR_11BITS_INTEGER 4

void inscription(char *argv[]) {
    char *hostname = argv[1];
    char *port = argv[2];
    inscription_request(hostname, port);
}

void request(char *buf, char *argv[]) {
    printf("CHOIX REQUETE : \n <2> POST BILLET \n <3> DEMANDER N BILLETS \n <4> ABONNEMENTS FIL \n <5> AJOUTER UN FICHIER \n <6> TELECHARGER UN FICHIER \n");
    memset(buf, 0, SIZE_MESS);
    fgets(buf, SIZE_MESS, stdin);

    char *hostname = argv[1];
    char *port = argv[2];
    
    uint8_t codereq_client = atoi(buf);

    switch (codereq_client)
    {
    case REQ_POST_BILLET:
        post_billet_request(hostname, port);
        break;
    case REQ_GET_BILLET:
        get_billets_request(hostname, port);
        break;
    case REQ_SUBSCRIBE:
      subscribe_request(sock);
        break;
    case REQ_ADD_FILE:
        add_file_request(sock);
        break;
    case REQ_ADD_FILE:
        /*add_file_request();
        break;*/
    case REQ_DW_FILE:
        /*download_file_request();
        break;*/
    default:
        perror("ERREUR : Requete inconnue");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    printf("ÊTES-VOUS INSCRIT ? (o/n) :\n");
    char buf[SIZE_MESS];
    memset(buf, 0, SIZE_MESS);
    fgets(buf, SIZE_MESS, stdin);
    buf[strlen(buf)-1] = '\0';

    if (strcmp(buf, "o") != 0 && strcmp(buf, "n") != 0) {
        printf("ERREUR : Veuillez saisir 'o' ou 'n' \n");
        exit(1);
    }
    
    if (strcmp(buf, "o") == 0) {
        request(buf, argv);
    }
    else {
        inscription(argv);
    }
}