#include <arpa/inet.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "func/func_client.h"
#include "request.h"
#include "interface_client.h"

#define BUF_SIZE 200

static void loop(int sock, uint16_t id) {
    bool quit = false;
    while (!quit) {
        char buf[BUF_SIZE];
        ask("[P]ost billet\n"
            "[D]emander N billets\n"
            "a[B]onnements fil\n"
            "[A]jouter un fichier\n"
            "[T]elecharger un fichier\n"
            "[Q]uitter\n"
            "[pdbatq] ",
            buf, BUF_SIZE);

        switch (getCharacter(buf)) {
        case 'p':
            post_billet_request(sock, id);
            break;
        case 'd':
            get_billets_request(sock, id);
            break;
        case 'b':
            subscribe_request(sock, id);
            break;
        case 'a':
            add_file_request(sock, id);
            break;
        case 't':
            dw_file_request(sock, id);
            break;
        case 'q':
            quit = true;
            break;
        }
    }
}

static int inscription(int sock) {
    char buf[BUF_SIZE];

    username_t username;
    username_error r;
    bool done = false;
    while (!done) {
        ask("Nom d'utilisateur ", buf, BUF_SIZE);
        r = string_to_username(buf, username);

        switch(r) {
        case USERNAME_EMPTY:
            fprintf(stderr, "Nom d'utilisateur vide.\n");
            break;
        case USERNAME_TOO_LONG:
            fprintf(stderr, "Nom d'utilisateur limité a 10 charactères.\n");
            break;
        case USERNAME_OK:
            done = true;
            break;
        }
    }

    u_int16_t id = inscription_request(sock, username);

    printf("Identifiant: %d\n\n", id);

    return id;
}

static int connexion() {
    char buf[BUF_SIZE];

    int n;
    bool done = false;
    while(!done) {
        ask("Identifiant ", buf, BUF_SIZE);

        n = atoi(buf);

        if (n >= 0 && n < (1 << 11)) {
            done = true;
        } else {
            fprintf(stderr, "Identifiant invalide\n");
        }
    }

    return n;
}

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    const char *port = argv[2];
    int sock = connexion_server(hostname, port);

    uint16_t id = 0;
    bool done = false;

    while(!done) {
        char buf[BUF_SIZE] = {0};
        ask("[I]nscription\n"
            "[C]onnexion\n"
            "[ic] ",
            buf, BUF_SIZE);

        switch (getCharacter(buf)) {
        case 'i':
            id = inscription(sock);
            done = true;
            break;
        case 'c':
            id = connexion();
            done = true;
            break;
        }
    }

    loop(sock, id);
    close(sock);
}
