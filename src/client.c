#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "../headers/socket.h"
#include "../headers/billet.h"

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

	return strcmp(buf, "o") == 0 ? 1 : 0;
}

//connexion du client au serveur
void connexion(int sock){
  char username[MAX_USERNAME_LEN+1];
  int user_id;
  char id_str[12];


  //Reception de la demande de connexion
  char msg[SIZE_MESS];
  if(recv(sock, msg, SIZE_MESS, 0) <= 0){
    perror("erreur lecture");
    exit(4);
  }
  printf("%s\n", msg);
  nettoyage();
  //Saisie du pseudo
  fgets(username, MAX_USERNAME_LEN+1, stdin);
  strtok(username, "\n");
  //envoie pseud
  if(send(sock, username, strlen(username), 0) <= 0){
    perror("erreur ecriture");
    exit(3);
  }
  //reception de la saisie  l'id
  
  char msg2[SIZE_MESS];
  
  
  if(recv(sock, msg2, SIZE_MESS, 0) <= 0){
    perror("erreur lecture");
    exit(4);
  }
  
  printf("%s\n", msg2);
  nettoyage();
  //saisie de l'id 
  fgets(id_str, 11, stdin);
  strtok(id_str, "\n"); 
  //envoie de l'id
  
  if(send(sock, id_str, 11, 0) <= 0){
    perror("erreur ecriture");
    exit(3);
  }

close(sock);

}




void inscription(int sock) {
    char username[MAX_USERNAME_LEN+1];
    int user_id=0;
    char id_str[12];

    // Reception de la demande d'inscription
    char msg[SIZE_MESS];
    int recu = recv(sock, msg, SIZE_MESS, 0);
    if (recu <= 0){
      perror("erreur lecture");
      exit(4);
    }
    printf("%s\n", msg);
    // !!!!!!!!!!!!
    nettoyage();

    // Saisie du pseudo
    fgets(username, MAX_USERNAME_LEN+1, stdin);
    strtok(username, "\n");

    // Envoi du pseudo
    int ecrit = send(sock, username, strlen(username), 0);
    if(ecrit <= 0){
      perror("erreur ecriture");
      exit(3);
    }

    // Reception de l'id
    recu = recv(sock, id_str, 12, 0);
    if (recu <= 0){
      perror("erreur lecture");
      exit(4);
    }

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
