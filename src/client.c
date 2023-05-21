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

#define BUF_SIZE 200

static int ask(const char *question, char *answer_buf, size_t size) {
    printf("%s> ", question);

    char *r = fgets(answer_buf, size, stdin);
    if (r == NULL) exit(0); // EOF

    printf("\n");

    return 0;
}

static void loop(int sock, uint16_t id) {
    bool quit = false;
    while (!quit) {
        char buf[BUF_SIZE];
        ask("[P]ost billet\n"
            "[D]emander N billets\n"
            "a[B]onnements fil\n"
            "[A]jouter un fichier\n"
            "[T]elecharger un fichier\n"
            "[Q]uitter\n",
            buf, BUF_SIZE);

        char c = buf[0];
        if (c == 'p' || c == 'P') {
            post_billet_request(sock, id);
        } else if (c == 'd' || c == 'D') {
            get_billets_request(sock, id);
        } else if (c == 'b' || c == 'B') {
            subscribe_request(sock, id);
        } else if (c == 'a' || c == 'A') {
            add_file_request(sock, id);
        } else if (c == 't' || c == 'T') {
            dw_file_request(sock, id);
        } else if (c == 'q' || c == 'Q') {
            quit = true;
        }
    }
}

static int inscription(int sock) {
    char buf[BUF_SIZE];

    username_t username;
    username_error r;
    do {
        ask("Nom d'utilisateur ", buf, BUF_SIZE);
        r = string_to_username(buf, username);

        if (r == USERNAME_EMPTY) {
            fprintf(stderr, "Nom d'utilisateur vide.\n");
        } else if (r == USERNAME_TOO_LONG) {
            fprintf(stderr, "Nom d'utilisateur limité a 10 charactères.\n");
        }
    } while (r != USERNAME_OK);

    u_int16_t id = inscription_request(sock, username);

    printf("Identifiant: %d\n", id);

    return id;
}

static int connexion() {
    char buf[BUF_SIZE];

    int n;
    do {
        ask("Identifiant ", buf, BUF_SIZE);

        n = atoi(buf);

        if (n >= 0 && n < (1 << 11)) {
            return n;
        } else {
            fprintf(stderr, "Identifiant invalide\n");
        }

    } while(1);
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
    do {
        char buf[BUF_SIZE] = {0};
        ask("[I]nscription\n"
            "[C]onnexion\n",
            buf, BUF_SIZE);

        char c = buf[0];
        if (c == 'i' || c == 'I') {
            id = inscription(sock);
        } else if (c == 'c' || c == 'C') {
            id = connexion();
        }
    } while(id == 0);

    loop(sock, id);
    close(sock);
}
