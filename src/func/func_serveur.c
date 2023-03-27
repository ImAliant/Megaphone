#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "../../headers/func/func_serveur.h"

#define BITS_ID 11
#define MAX_USERNAME_LEN 10
#define SIZE_MESS 100

void remove_special_chars(char *str) {
    int i, j;
    for (i = 0, j = 0; str[i]; i++) {
        if (!isspace(str[i]) && isprint(str[i])) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

void demande_client_pseudo(int sock, char *message, char *username) {
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
}

void demande_client_id(int sock, char *id_str) {
    char *message = "Entrez votre id :";
    if(send(sock, message, SIZE_MESS, 0) <= 0){
        perror("send");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }
    //reception de l'id du client
    int recu = recv(sock, id_str, BITS_ID+1, 0);
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
}

void find_pseudo_id_in_list(char *id_str, char *username, int sock, int nb_utilisateurs, utilisateur liste[]) {
    int i;
    int user_id = atoi(id_str);
    for(i=0; i<nb_utilisateurs; i++){
        if((strcmp(liste[i].pseudo, username) == 0 )&&(liste[i].id == user_id)){
            printf("Connexion reussie\n");
            break;
        }
        else{
            printf("Connexion echouee\n");
            close(sock);
            exit(1);
        }
    }
}

void completion_pseudo(char *username) {
    int len = strlen(username);
    if (len < MAX_USERNAME_LEN) {
        memset(username+len, '#', MAX_USERNAME_LEN-len);
        username[MAX_USERNAME_LEN] = '\0';
        remove_special_chars(username);
    }
}

int demande_client_insc_conn(int sock) {
    char c;
    int ecrit = send(sock, "INSCRIPTION/CONNEXION (i/c) ?", SIZE_MESS, 0);
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

    return c=='i'?1:0;
}

void envoi_id_client(int sock, int user_id) {
    char id_str[12];
    snprintf(id_str, 12, "%11d", user_id);
    int ecrit = send(sock, id_str, 11, 0);
    if(ecrit <= 0) {
        perror("send");
        close(sock);
        int *ret = malloc(sizeof(int));
        *ret = 1;
        pthread_exit(ret);
    }
}