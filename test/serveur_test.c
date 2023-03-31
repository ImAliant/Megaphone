#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define SIZE_MESS 100
#define NUM_THREADS 100
#define MAX_USERS 100
#define MAX_USERNAME_LEN 10

typedef struct {
    char *pseudo;
    uint16_t id;
} utilisateur;

typedef struct {
    int sock;
    struct sockaddr_in6 addr;
} client_info;

int nb_utilisateurs = 0;

utilisateur liste[MAX_USERS];
client_info clients[MAX_USERS];

int generate_user_id() {
    static int current_user_id = 0;
    current_user_id++;
    current_user_id &= 0x7FF;
    return current_user_id;
}

void create_new_user(char *username, int user_id) {
    utilisateur user;
    user.pseudo = malloc(sizeof(char*)*MAX_USERS);
    strcpy(user.pseudo, username);
    user.id = user_id;

    liste[nb_utilisateurs] = user;
    nb_utilisateurs++;
}

void remove_special_chars(char *str) {
    int i, j;
    for (i = 0, j = 0; str[i]; i++) {
        if (!isspace(str[i]) && isprint(str[i])) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

void completion_pseudo(char *username) {
    int len = strlen(username);
    if (len < MAX_USERNAME_LEN) {
        memset(username+len, '#', MAX_USERNAME_LEN-len);
        username[MAX_USERNAME_LEN] = '\0';
        remove_special_chars(username);
    }
}

int main(int argc, char *argv[]) {
    int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in6 servadr;
    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    servadr.sin6_addr = in6addr_any;
    servadr.sin6_port = htons(atoi(argv[1]));

    int r = bind(sock, (struct sockaddr *) &servadr, sizeof(servadr));
    if (r < 0) return -1;

    char buffer[SIZE_MESS+1];
    struct sockaddr_in6 cliadr;
    socklen_t len = sizeof(cliadr);

    // RECEPTION D'UN CARACTER QUI INDIQUE SI LE CLIENT S'EST DEJA CONNECTE AUPARAVANT AU SERVEUR
    while (1) {
        memset(buffer, 0, SIZE_MESS+1);
        r = recvfrom(sock, buffer, SIZE_MESS, 0, (struct sockaddr *) &cliadr, &len);
        if (r < 0) return -1;
        printf("message recu - %d octets : %s\n", r, buffer);

        int connected_before = buffer[0] == '0' ? 0 : 1;
        printf("connected_before = %d\n", connected_before);

        memset(buffer, 0, SIZE_MESS+1);
        sprintf(buffer, "%s", connected_before ? "CONNEXION (ENTRE VOTRE IDENTIFIANT)" : "INSCRIPTION (ENTRE UN PSEUDO AVEC MOINS DE 10 CARACTERES)");
        r = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &cliadr, len);
        if (r < 0) return -1;

        // RECEPTION DU PSEUDO OU DE L'ID
        memset(buffer, 0, SIZE_MESS+1);
        r = recvfrom(sock, buffer, MAX_USERNAME_LEN, 0, (struct sockaddr *) &cliadr, &len);
        if (r < 0) return -1;

        if (connected_before == 0) {
            // INSCRIPTION
            uint16_t header_client;
            r = recvfrom(sock, &header_client, sizeof(header_client), 0, (struct sockaddr *) &cliadr, &len);
            if (r < 0) return -1;
            uint8_t type = ntohs(header_client) & 0x1F;

            memset(buffer, 0, SIZE_MESS+1);
            r = recvfrom(sock, buffer, SIZE_MESS, 0, (struct sockaddr *) &cliadr, &len);
            if (r < 0) return -1;

            completion_pseudo(buffer);
            int user_id = generate_user_id();
            create_new_user(buffer, user_id);

            uint16_t header_server = htons((user_id << 5) | (type & 0x1F));
            r = sendto(sock, &header_server, sizeof(header_server), 0, (struct sockaddr *) &cliadr, len);
            if (r < 0) return -1;
        } else {
            // CONNEXION
            // TODO
        }
    }

    // RECEPTION PAQUET UDP
    /*uint16_t header_client;
    r = recvfrom(sock, &header_client, sizeof(header_client), 0, (struct sockaddr *) &cliadr, &len);
    if (r < 0) return -1;
    uint8_t type = ntohs(header_client) & 0x1F;
    uint16_t id = ntohs(header_client) >> 5;

    memset(buffer, 0, SIZE_MESS+1);
    r = recvfrom(sock, buffer, SIZE_MESS, 0, (struct sockaddr *) &cliadr, &len);
    if (r < 0) return -1;

    if (type == 1) {
        // INSCRIPTION
        int user_id = generate_user_id();
        create_new_user(buffer, user_id);

        uint16_t header_server = htons((user_id << 5) | (type & 0x1F));
        r = sendto(sock, &header_server, sizeof(header_server), 0, (struct sockaddr *) &cliadr, len);
        if (r < 0) return -1;
    } else if (type == 2) {
        // CONNEXION
        // TODO
    }*/

    close(sock);

    return 0;
}