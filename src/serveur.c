#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <ctype.h>

#include "../headers/socket.h"
#include "../headers/billet.h"
#include "../headers/func/func_serveur.h"
#include "../headers/users.h"

#define MAX_USERNAME_LEN 10
#define SIZE_MESS 100
#define SIZE_MAX_LISTE 100

int nb_utilisateurs = 0;

utilisateur liste[SIZE_MAX_LISTE];

void recep_udp_message(uint16_t header) {
    uint8_t type = ntohs(header) & 0x1F;
    uint16_t client_id = ntohs(header) >> 5;
}

int generate_user_id() {
    static int current_user_id = 0;
    current_user_id++;
    current_user_id &= 0x7FF;
    return current_user_id;
}

void create_new_user(char *username, int user_id) {
    liste[nb_utilisateurs].pseudo = malloc(sizeof(char*)*MAX_USERNAME_LEN);
    strcpy(liste[nb_utilisateurs].pseudo, username);
    liste[nb_utilisateurs].id = user_id;
    nb_utilisateurs++;
}

void inscription(int sock) {
    char username[MAX_USERNAME_LEN+1];
    int user_id;

    // Demande au client de s'inscrire
    char *message = "Entrez votre pseudo (max 10 caracteres):";
    demande_client_pseudo(sock, message, username);
    
    // Completion du pseudo avec des #
    completion_pseudo(username);

    user_id = generate_user_id();
    create_new_user(username, user_id);

    // Envoi de l'id au client
    envoi_id_client(sock, user_id);

    close(sock);
}

void connexion(int sock, int nb_utilisateurs, utilisateur liste[]) {
    char username[MAX_USERNAME_LEN+1];
    char id_str[12];
    
    char *message = "Entrez votre pseudo :";
    demande_client_pseudo(sock, message, username);
    
    completion_pseudo(username);

    demande_client_id(sock, id_str);
    
    find_pseudo_id_in_list(id_str, username, sock, nb_utilisateurs, liste);

    close(sock);
}

void *serve(void *arg) {
    int sock = *((int *) arg);
    char buf[SIZE_MESS];
    memset(buf, 0, sizeof(buf));

    // On demande a l'utilisateur si il veut s'inscrire ou se connecter
    int r = demande_client_insc_conn(sock);
    
    // On traite le cas ou l'utilisateur veut s'inscrire
    // On recupere le pseudo
    if (r == 1) {
        inscription(sock);
    }
    else if (r == 0) {
        connexion(sock, nb_utilisateurs, liste);
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
    int port, sock;

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

    close(sock);
    
    return 0;
}
