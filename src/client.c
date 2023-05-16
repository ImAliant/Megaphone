#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "func/func_client.h"
#include "request.h"

#define SIZE_MESS 200
#define MAX_USERNAME_LEN 10
#define SIZE_STR_11BITS_INTEGER 4

static void request(int sock) {
    printf("CHOIX REQUETE :\n"
           "<2> POST BILLET\n"
           "<3> DEMANDER N BILLETS\n"
           "<4> ABONNEMENTS FIL\n"
           "<5> AJOUTER UN FICHIER\n"
           "<6> TELECHARGER UN FICHIER\n");

    char buf[SIZE_MESS + 1] = {0};
    char *r = fgets(buf, SIZE_MESS, stdin);
    if (r == NULL) {
        fprintf(stderr, "Erreur : EOF\n");
        exit(1);
    }

    codereq_t codereq_client = atoi(buf);

    switch (codereq_client) {
    case REQ_POST_BILLET:
        post_billet_request(sock);
        break;
    case REQ_GET_BILLET:
        get_billets_request(sock);
        break;
    case REQ_SUBSCRIBE:
    case REQ_ADD_FILE:
    case REQ_DW_FILE:
        fprintf(stderr, "%s:%d: TODO\n", __FILE__, __LINE__);
        exit(1);
    default:
        fprintf(stderr, "ERREUR : Requete inconnue");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    char *hostname = argv[1];
    char *port = argv[2];
    int sock = connexion_server(hostname, port);

    printf("ÊTES-VOUS INSCRIT ? (o/n) :\n");
    char buf[SIZE_MESS];
    memset(buf, 0, SIZE_MESS);
    char *r = fgets(buf, SIZE_MESS, stdin);
    if (r == NULL) {
        fprintf(stderr, "Erreur : EOF\n");
        exit(1);
    }

    buf[strlen(buf) - 1] = '\0';

    if (strcmp(buf, "o") != 0 && strcmp(buf, "n") != 0) {
        printf("ERREUR : Veuillez saisir 'o' ou 'n' \n");
        exit(1);
    }

    if (strcmp(buf, "o") == 0) {
        request(sock);
    } else {
        inscription_request(sock);
    }
}
