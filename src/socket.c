#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "socket.h"

int creation_socket() {
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if(sock < 0){
      perror("creation socket");
      exit(1);
    }

    return sock;
}

int accept_connexion(int sock, struct sockaddr_in6 adr, socklen_t lg) {
    int sock_client = accept(sock, (struct sockaddr *)&adr, &lg);
    if(sock_client < 0){
        perror("accept");
        exit(1);
    }

    return sock_client;
}

void addresse_destinataire(int port, struct sockaddr_in6 *address_sock){
    address_sock->sin6_family = AF_INET6;
    address_sock->sin6_port = htons(port);
    address_sock->sin6_addr = in6addr_any;
}

void desac_option_only_ipv6(int sock) {
    int r, optval = 0;
    r = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
    if (r < 0) 
        perror("erreur connexion IPv4 impossible");
}

void parallel_use_port(int sock) {
    int r;
    int optval = 1;
    r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (r < 0) 
        perror("erreur réutilisation de port impossible");
}

void bind_port(int sock, struct sockaddr_in6 *address_sock) {
    int r;
    r = bind(sock, (struct sockaddr *) address_sock, sizeof(*address_sock));
    if (r < 0) {
        perror("erreur bind");
        exit(2);
    }
}

void listen_port(int sock) {
    int r;
    r = listen(sock, 0);
    if (r < 0) {
        perror("erreur listen");
        exit(2);
    }
}

void affiche_adresse(struct sockaddr_in6 *adr){
    char adr_buf[INET6_ADDRSTRLEN];
    memset(adr_buf, 0, sizeof(adr_buf));
    
    inet_ntop(AF_INET6, &(adr->sin6_addr), adr_buf, sizeof(adr_buf));
    printf("CONNEXION SERVEUR : IP: %s port: %d\n", adr_buf, ntohs(adr->sin6_port));
}

int get_server_addr(char* hostname, char* port, int * sock, struct sockaddr_in6** addr, int* addrlen) {
    struct addrinfo hints, *r, *p;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED | AI_ALL;

    if ((ret = getaddrinfo(hostname, port, &hints, &r) != 0) || NULL == r){
        fprintf(stderr, "erreur getaddrinfo : %s\n", gai_strerror(ret));
        return -1;
    }
    
    *addrlen = sizeof(struct sockaddr_in6);
    p = r;
    while( p != NULL ){
        affiche_adresse((struct sockaddr_in6 *) p->ai_addr);
        if((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) > 0){
            if(connect(*sock, p->ai_addr, *addrlen) == 0)
    	        break;

            close(*sock);
        }

        p = p->ai_next;
    }

    if (NULL == p) return -2;

    //on stocke l'adresse de connexion
    *addr = (struct sockaddr_in6 *) p->ai_addr;

    //on libère la mémoire allouée par getaddrinfo 
    freeaddrinfo(r);

    return 0;
}