#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "../headers/socket.h"
#include "../headers/billet.h"
#include "../headers/func/func_client.h"

#define SIZE_MESS 100
#define MAX_USERNAME_LEN 10

//connexion du client au serveur
void connexion(int sock){
    char username[MAX_USERNAME_LEN+1];
    char id_str[12];

    // Reception du message concernant la connexion
    recep_demande(sock);
    
    saisie_pseudo(username);
    
    envoie_pseudo(sock, username);
    
    // Reception du message concernant l'id
    recep_demande(sock);

    saisie_id(id_str);
    
    envoie_id(sock, id_str);

    close(sock);
}

void inscription(int sock) {
    char username[MAX_USERNAME_LEN+1];
    int user_id=0;
    char id_str[12];

    // Reception de la demande du pseudo
    recep_demande(sock);

    saisie_pseudo(username);

    envoie_pseudo(sock, username);

    recep_id(sock, id_str);

    user_id = atoi(id_str);

    printf("Inscription reussie ! Votre identifiant est %d\n", user_id);

    close(sock);
}

int main(int argc, char** args) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", args[0]);
        exit(1);
    }
    
    struct sockaddr_in6* server_addr;
    int fdsock, adrlen;
    
    switch (get_server_addr(args[1], args[2], &fdsock, &server_addr, &adrlen)) {
    case 0: printf("adresse creee !\n"); break;
    case -1:
      fprintf(stderr, "Erreur: hote non trouve.\n"); 
    case -2:
      fprintf(stderr, "Erreur: echec de creation de la socket.\n");
      exit(1);
    }

    // Le serveur nous envoie un message de bienvenue et nous demande si nous voulons nous inscrire ou nous connecter
    
    int r = rep_demande_inscription(fdsock);
    
	if (r == 1) {
        inscription(fdsock);
    }else if (r == 0) {
        connexion(fdsock);
    }else {
        fprintf(stderr, "Erreur: reponse incorrecte.\n");
        exit(1);
    }
    close(fdsock);
    
    return 0;
}
