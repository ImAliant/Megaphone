#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE_MESS 100
#define NOM "Cerise"

int main(int argc, char *argv[]){
    struct sockaddr_in6 address_sock, adrclient;
    int port, r, sock, sockclient, optval;
    socklen_t size;

    if(argc != 2){
      fprintf(stderr, "usage : ./serveur <PORT>\n");
      exit(1);
    }

    //*** creation de l'adresse du destinataire (serveur) ***
    port = atoi(argv[1]);
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(port);
    address_sock.sin6_addr = in6addr_any;

    //*** creation de la socket ***
    sock = socket(PF_INET6, SOCK_STREAM, 0);
    if(sock < 0){
      perror("creation socket");
      exit(1);
    }

    //*** desactiver l'option n'accepter que de l'IPv6 **
    optval = 0;
    r = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
    if (r < 0) 
      perror("erreur connexion IPv4 impossible");

    //*** le numero de port peut etre utilise en parallele ***
    optval = 1;
    r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (r < 0) 
      perror("erreur réutilisation de port impossible");

    //*** on lie la socket au port ***
    r = bind(sock, (struct sockaddr *) &address_sock, sizeof(address_sock));
    if (r < 0) {
      perror("erreur bind");
      exit(2);
    }

    //*** Le serveur est pret a ecouter les connexions sur le port ***
    r = listen(sock, 0);
    if (r < 0) {
      perror("erreur listen");
      exit(2);
    }

    //*** le serveur accepte une connexion et cree la socket de communication avec le client ***
    memset(&adrclient, 0, sizeof(adrclient));
    size=sizeof(adrclient);
    sockclient = accept(sock, (struct sockaddr *) &adrclient, &size);
    
    if (sockclient >= 0) {

      //*** affichage de l'adresse du client ***
      char nom_dst[INET6_ADDRSTRLEN];
      //printf("adresse client %s\n", inet_ntop(AF_INET6,&adrclient.sin6_addr,nom_dst,sizeof(nom_dst)));
      struct sockaddr_in6 adr2client;
      socklen_t size2 = sizeof(adr2client);
      if(getpeername(sockclient, (struct sockaddr *) &adr2client, &size2) == 0)
        printf("adresse client %s\n", inet_ntop(AF_INET6, &adr2client.sin6_addr, nom_dst, sizeof(nom_dst)));

      memset(nom_dst, 0, sizeof(nom_dst));
      struct sockaddr_in6 adrserv;
      socklen_t size3 = sizeof(adrserv);
      if(getsockname(sockclient, (struct sockaddr *) &adrserv, &size3) == 0)
        printf("adresse serveur %s\n", inet_ntop(AF_INET6, &adrserv.sin6_addr, nom_dst, sizeof(nom_dst)));

      //*** reception d'un message ***
      char buf[SIZE_MESS];
      memset(buf, 0, SIZE_MESS);
      int recu = recv(sockclient, buf, (SIZE_MESS-1) * sizeof(char), 0);
      if (recu <= 0){
        perror("erreur lecture");
        exit(4);
      }
      buf[recu] = '\0';
      printf("%s\n", buf);

      //*** envoie d'un message ***
      memset(buf, 0, SIZE_MESS);
      sprintf(buf, "Salut %s", NOM);
      int ecrit = send(sockclient, buf, strlen(buf), 0);
      if(ecrit <= 0){
        perror("erreur ecriture");
        exit(3);
      }
    }

    //*** fermeture socket client ***
    close(sockclient);

    //*** fermeture socket serveur ***
    close(sock);
    
    return 0;
}
