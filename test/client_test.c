#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SIZE_MESS 100

int main(int argc, char *argv[]) {
    int sock = socket(PF_INET6, SOCK_DGRAM, 0);
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
    sprintf(buffer, "TEST");

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

    close(sock);
}