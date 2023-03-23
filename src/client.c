#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "../headers/socket.h"

#define SIZE_MESS 100

/*void send_mess(int sock) {
    printf("Entrez votre message : ");
    char buf[SIZE_MESS+1];
    memset(buf, 0, SIZE_MESS);
    
    fgets(buf, SIZE_MESS, stdin);
    
    int ecrit = send(sock, buf, strlen(buf), 0);
    if(ecrit <= 0){
      perror("erreur ecriture");
      exit(3);
    }
}

void recv_mess(int sock) {
    char buf[SIZE_MESS];
    memset(buf, 0, SIZE_MESS+1);
    int recu = recv(sock, buf, SIZE_MESS, 0);
    if (recu <= 0){
      perror("erreur lecture");
      exit(4);
    }
    buf[recu] = '\0';
    printf("%s\n", buf);
}*/

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
    char buf[SIZE_MESS];
    int recu = recv(fdsock, buf, SIZE_MESS, 0);
    if (recu <= 0){
      perror("erreur lecture");
      exit(4);
    }
    buf[recu] = '\0';
    printf("%s\n", buf);

    memset(buf, 0, SIZE_MESS);
    fgets(buf, 2, stdin);

    int ecrit = send(fdsock, buf, strlen(buf), 0);
    if(ecrit <= 0){
      perror("erreur ecriture");
      exit(3);
    }
    
    if (strcmp(buf, "o") == 0) {
        // Le serveur nous demande de nous inscrire
        memset(buf, 0, SIZE_MESS);
        printf("Entrez votre nom d'utilisateur (10 caractères maximum): ");
        fgets(buf, SIZE_MESS, stdin);
    
        ecrit = send(fdsock, buf, strlen(buf), 0);
        if(ecrit <= 0){
            perror("erreur ecriture");
            exit(3);
        }

        memset(buf, 0, SIZE_MESS);
        // Reception de l'identifiant
        recu = recv(fdsock, buf, SIZE_MESS, 0);
        if (recu <= 0){
            perror("erreur lecture");
            exit(4);
        }
        buf[recu] = '\0';
        printf("Votre identifiant : %s\n", buf);
    }

    close(fdsock);
    
    return 0;
}
