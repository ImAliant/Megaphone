#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "../headers/socket.h"
#include "../headers/billet.h"

#define MAX_USERNAME_LEN 10
#define SIZE_MESS 100
#define SIZE_MAX_LISTE 100
#define NOM "Cerise"

int nb_utilisateurs = 0;

//LISTE DES INSCRITS
typedef struct {
    char pseudo[MAX_USERNAME_LEN];
    uint16_t id;
} utilisateur;

utilisateur liste[SIZE_MAX_LISTE];



int generate_user_id() {
    static int current_user_id = 0;
    current_user_id++;
    current_user_id &= 0x7FF;
    return current_user_id;
}

void create_new_user(char *username, int user_id) {
    strcpy(liste[nb_utilisateurs].pseudo, username);
    liste[nb_utilisateurs].id = user_id;
    nb_utilisateurs++;
}

int demande_inscription(int sock) {
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
    }
    printf("Reponse inscription recu : %c\n", c);

    return c=='o'?1:0;
}

// TODO :fonction qui gere la connexion d'un client deja inscrit 
void connexion(int sock){
    char username[MAX_USERNAME_LEN+1];
    int user_id;
    char id_str[12];
    // Demande au client de se connecter
    char * message = "Entrez votre pseudo :";
    if(send(sock, message, strlen(message), 0) <= 0){
        perror("send");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }
    //reception du pseudo du client
    int recu = recv(sock, username, MAX_USERNAME_LEN, 0);
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
    }
    printf("Pseudo recu : %s\n", username);
    
    // Completion du pseudo avec des #
    int len = strlen(username);
    if (len < MAX_USERNAME_LEN) {
        memset(username+len, '#', MAX_USERNAME_LEN-len);
        username[MAX_USERNAME_LEN] = '\0';
    }
    // demande de l'id du client
    char * message2 = "Entrez votre id :";
    if(send(sock, message2, SIZE_MESS, 0) <= 0){
        perror("send");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }
    //reception de l'id du client
    recu = recv(sock, id_str, 12, 0);
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
    }
    printf("Id recu : %s\n", id_str);
    
    //TODO: verfication to be fixed ! verification de l'id et du pseudo
    int i;
    user_id = atoi(id_str);
    for(i=0; i<nb_utilisateurs; i++){
        if((strcmp(liste[i].pseudo, username) == 0 )&& (liste[i].id == user_id)){
            printf("Connexion reussie\n");
            break;
        }
        else{
            printf("Connexion echouee\n");
            close(sock);
            exit(1);
        }
    }

close(sock);




}





void inscription(int sock) {
    char username[MAX_USERNAME_LEN+1];
    int user_id;

    // Demande au client de s'inscrire
    char *message = "Entrez votre pseudo (max 10 caracteres):";
    int ecrit = send(sock, message, strlen(message), 0);
    if(ecrit <= 0) {
        perror("send");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }

    // Reception du pseudo du client
    int recu = recv(sock, username, MAX_USERNAME_LEN, 0);
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
        return;
    }
    printf("Pseudo recu : %s\n", username);
    
    
    // Completion du pseudo avec des #
    int len = strlen(username);
    if (len < MAX_USERNAME_LEN) {
        memset(username+len, '#', MAX_USERNAME_LEN-len);
        username[MAX_USERNAME_LEN] = '\0';
    }

    user_id = generate_user_id();
    create_new_user(username, user_id);

    char id_str[12];
    snprintf(id_str, 12, "%11d", user_id);
    ecrit = send(sock, id_str, 11, 0);
    if(ecrit <= 0) {
        perror("send");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }

    close(sock);
}

void *serve(void *arg) {
    int sock = *((int *) arg);
    char buf[SIZE_MESS];
    memset(buf, 0, sizeof(buf));

    // On demande a l'utilisateur si il veut s'inscrire ou se connecter
    int r = demande_inscription(sock);
    
    // On traite le cas ou l'utilisateur veut s'inscrire
    // On recupere le pseudo
    if (r == 1) {
        inscription(sock);
    }
    else if (r == 0) {
        connexion(sock);
        

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
