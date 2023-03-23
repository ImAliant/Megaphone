#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "../headers/socket.h"

#define SIZE_PSEUDO 10
#define SIZE_MESS 100
#define SIZE_MAX_LISTE 100
#define NOM "Cerise"

int nb_utilisateurs = 0;

//LISTE DES INSCRITS
typedef struct {
    char pseudo[SIZE_PSEUDO];
    uint16_t id;
} utilisateur;

utilisateur liste[SIZE_MAX_LISTE];

char demande_inscription(int sock) {
    char c;
    int ecrit = send(sock, "Voulez-vous vous inscrire (o/n) ?", 33, 0);
    if(ecrit <= 0)
        perror("send");
    int recu = recv(sock, &c, 1, 0);
    if (recu < 0){
        perror("recv");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }
    if(recu == 0){
        fprintf(stderr, "send du client nul\n");
        close(sock);
        return NULL;
    }
    printf("recu : %c\n", c);

    return c;
}

void inscription(int sock) {
    char buf[SIZE_MESS];
    memset(buf, 0, sizeof(buf));
    int recu = recv(sock, buf, SIZE_PSEUDO, 0);
    if (recu < 0){
        perror("recv");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }
    if(recu == 0){
        fprintf(stderr, "send du client nul\n");
        close(sock);
        return NULL;
    }
    printf("recu : %s\n", buf);
    // On verifie que le pseudo n'est pas deja pris
    int i;
    for(i = 0; i < nb_utilisateurs; i++) {
        if(strcmp(liste[i].pseudo, buf) == 0) {
            int ecrit = send(sock, "Pseudo deja pris", 16, 0);
            if(ecrit <= 0)
                perror("send");
            close(sock);
            int *ret = malloc(sizeof(int));
            *ret = 1;
            pthread_exit(ret);
        }
    }
    // L'identifiant est codé sur 11 bits
    utilisateur u;
    strcpy(u.pseudo, buf);
    u.id = nb_utilisateurs;
    liste[nb_utilisateurs] = u;
    nb_utilisateurs++;
}

void *serve(void *arg) {
    int sock = *((int *) arg);
    char buf[SIZE_MESS];
    memset(buf, 0, sizeof(buf));

    // On demande a l'utilisateur si il veut s'inscrire ou se connecter
    char c = demande_inscription(sock);
    
    // On traite le cas ou l'utilisateur veut s'inscrire
    // On recupere le pseudo
    if (c == 'o') {
        inscription(sock);
    }
    else if (c == 'n') {

    }
    else {
        fprintf(stderr, "Erreur de saisie\n");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }

    return NULL;
}

int loop_connexion_communication(struct sockaddr_in6 adrclient, int sock) {
    while(1) {
        memset(&adrclient, 0, sizeof(adrclient));
        socklen_t size=sizeof(adrclient);

        //*** On crée la variable sur le tas ***
        int *sockclient = malloc(sizeof(int));

        //*** Le serveur accepte une connexion et initialise la socket de communication avec le client ***
        *sockclient = accept(sock, (struct sockaddr *) &adrclient, &size);

        if (sockclient >= 0) {
            pthread_t thread;

            //*** Le serveur crée un thread et passe un pointeur sur socket client à la fonction serve ***
            if (pthread_create(&thread, NULL, serve, sockclient) == -1) {
                perror("pthread_create");
                continue;
            }
            //*** Affichage de l'adresse du client ***
            char nom_dst[INET6_ADDRSTRLEN];
            const char *inet = inet_ntop(AF_INET6, &adrclient.sin6_addr, nom_dst, sizeof(nom_dst));
            int port = htons(adrclient.sin6_port);
            printf("client connecte : %s %d\n", inet, port);
        }
    }
}
 
int main(int argc, char *argv[]){
    struct sockaddr_in6 address_sock, adrclient;
    int port, r, sock, sockclient;
    socklen_t size;

    if(argc != 2){
        fprintf(stderr, "usage : ./serveur <PORT>\n");
        exit(1);
    }

    port = atoi(argv[1]);
    addresse_destinataire(port, &address_sock);

    sock = creation_socket();

    desac_option_only_ipv6(sock);

    parallel_use_port(sock);

    bind_port(sock, &address_sock);

    listen_port(sock);

    loop_connexion_communication(adrclient, sock);

    //*** fermeture socket serveur ***
    close(sock);
    
    return 0;
}
