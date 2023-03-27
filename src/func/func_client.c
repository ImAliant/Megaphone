#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SIZE_MESS 100
#define MAX_USERNAME_LEN 10

void nettoyage() {
    fflush(stdout);
    fflush(stdin);

    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

int rep_demande_inscription(int fdsock) {
    char buf[SIZE_MESS];
    int recu = recv(fdsock, buf, SIZE_MESS+1, 0);
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

	return strcmp(buf, "i") == 0 ? 1 : 0;
}

void recep_demande(int sock) {
    char msg[SIZE_MESS];
    int recu = recv(sock, msg, SIZE_MESS+1, 0);
    if (recu <= 0){
      perror("erreur lecture");
      exit(4);
    }
    printf("%s\n", msg);
    nettoyage();
}

void envoie_pseudo(int sock, char *username) {
    int ecrit = send(sock, username, strlen(username), 0);
    if(ecrit <= 0){
      perror("erreur ecriture");
      exit(3);
    }
}

void envoie_id(int sock, char *id_str) {
    int ecrit = send(sock, id_str, 11, 0);
    if(ecrit <= 0){
      perror("erreur ecriture");
      exit(3);
    }
}

void saisie_pseudo(char *username) {
    fgets(username, MAX_USERNAME_LEN+1, stdin);
    strtok(username, "\n");
}

void saisie_id(char *id_str) {
    fgets(id_str, 11, stdin);
    strtok(id_str, "\n");
}

void recep_id(int sock, char *id_str) {
    int recu = recv(sock, id_str, 12, 0);
    if (recu <= 0){
      perror("erreur lecture");
      exit(4);
    }
}