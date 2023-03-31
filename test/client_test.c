#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SIZE_MESS 100
#define MAX_USERNAME_LEN 10

#define BUF_LEN 26 // 16 bytes for header + 10 bytes for username

int main(int argc, char *argv[]) {
    /*int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) return -1;

    // adresse de destination
    struct sockaddr_in6 servadr;
    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, argv[1], &servadr.sin6_addr);
    servadr.sin6_port = htons(atoi(argv[2]));
    socklen_t len = sizeof(servadr);

    char buffer[SIZE_MESS];
    memset(buffer, 0, SIZE_MESS);
    // 0 : inscription
    // 1 : connexion
    fgets(buffer, SIZE_MESS, stdin);

    int r = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &servadr, len);
    if (r < 0) {
        perror("sendto");
        return -1;
    }

    memset(buffer, 0, SIZE_MESS);
    r = recvfrom(sock, buffer, SIZE_MESS, 0, (struct sockaddr *) &servadr, &len);
    if (r < 0) {
        perror("recvfrom");
        return -1;
    }
    printf("message recu - %d octets : %s\n", r, buffer);

    // ENVOIE DU PSEUDO
    uint16_t id = 0;
    uint8_t type = 1;
    uint16_t header_client = htons((id << 5) | (type & 0x1F));
    r = sendto(sock, &header_client, sizeof(header_client), 0, (struct sockaddr *) &servadr, len);
    memset(buffer, 0, SIZE_MESS);
    fgets(buffer, MAX_USERNAME_LEN, stdin);
    r = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &servadr, len);

    // RECEPTION DE L'ID
    uint16_t header_server;
    r = recvfrom(sock, &header_server, sizeof(header_server), 0, (struct sockaddr *) &servadr, &len);
    if (r < 0) {
        perror("recvfrom");
        return -1;
    }
    id = ntohs(header_server) >> 5;
    type = ntohs(header_server) & 0x1F;

    printf("id : %d, type : %d\n", id, type);

    close(sock);*/

    int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur socket");
        return -1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in6 servadr;
    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip, &servadr.sin6_addr);
    servadr.sin6_port = htons(port);
    socklen_t len = sizeof(servadr);

    char buffer[BUF_LEN];
    
    //entete
    uint16_t header;
    uint16_t id = 0;
    uint8_t type = 1;
    header = htons((id << 5) | (type & 0x1F));

    // pseudo
    char username[MAX_USERNAME_LEN+1];
    memset(username, 0, MAX_USERNAME_LEN+1);
    fgets(username, MAX_USERNAME_LEN, stdin);

    // copie de l'entete et du pseudo dans le buffer
    memcpy(buffer, &header, sizeof(header));
    memcpy(buffer+sizeof(header), username, strlen(username));

    // envoi de l'entete
    int r = sendto(sock, buffer, sizeof(header), 0, (struct sockaddr *) &servadr, len);
    if (r < 0) {
        perror("sendto");
        return -1;
    }

    // reception reponse serveur
    memset(buffer, 0, BUF_LEN);
    r = recvfrom(sock, buffer, BUF_LEN, 0, (struct sockaddr *) &servadr, &len);
    if (r < 0) {
        perror("recvfrom");
        return -1;
    }

    // recuperation de l'id
    memcpy(&header, buffer, sizeof(header));
    id = ntohs(header) >> 5;

    printf("id : %d\n", id);

    // terminaison de la connexion
    close(sock);
}