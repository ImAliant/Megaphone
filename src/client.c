#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#include "../headers/socket.h"
#include "../headers/request_definition.h"
#include "../headers/func/func_client.h"

#define SIZE_MESS 100
#define MAX_USERNAME_LEN 10

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
        printf("CHOIX REQUETE : \n <2> POST BILLET \n <3> DEMANDER N BILLETS \n <4> ABONNEMENTS FIL \n <5> AJOUTER UN FICHIER \n <6> TELECHARGER UN FICHIER \n");
        memset(buf, 0, SIZE_MESS);
        fgets(buf, SIZE_MESS, stdin);
        
        uint8_t codereq_client = atoi(buf);

        switch (codereq_client)
        {
        case REQ_POST_BILLET:
        case REQ_GET_BILLET:
        case REQ_SUBSCRIBE:
        case REQ_ADD_FILE:
        case REQ_DW_FILE:
            break;
        }
    }
    else {
        char *hostname = argv[1];
        char *port = argv[2];
        inscription_request(hostname, port);
    }
}