#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 7777
#define ADDR "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"

void address_dest(struct sockaddr_in address) {
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    inet_pton(AF_INET, "192.168.70.200", &address.sin_addr);
}

int main() {
    // creation de la socket
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("creation socket");
        exit(1);   
    }

    // creation de l'adresse du destinataire (serveur)
    struct sockaddr_in address_sock;
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