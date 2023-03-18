#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 7777
#define ADDR "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"

void address_dest(struct sockaddr_in6 address) {
    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(PORT);
    inet_pton(AF_INET6, ADDR, &address.sin6_addr);
}

int main() {
    // creation de la socket
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("creation socket");
        exit(1);   
    }

    // creation de l'adresse du destinataire (serveur)
    struct sockaddr_in6 address_sock;
    address_dest(address_sock);

    // demande de connexion au serveur
    int r = connect(sock, (struct sockaddr *)&address_sock, sizeof(address_sock));
    if (r == -1) {
        perror("echec de la connexion");
        close(sock);
        exit(2);
    }
    printf("connexion etablie avec le serveur\n");

    // envoi (TODO)

    // reception (TODO)

    // fermeture de la socket
    close(sock);
    printf("connexion terminee\n");

    return 0;
}