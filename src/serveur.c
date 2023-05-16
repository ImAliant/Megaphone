#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "billet.h"
#include "error.h"
#include "func/func_serveur.h"
#include "message.h"
#include "request.h"
#include "users.h"

#define SIZE_MESS 200
#define MAX_USERS 100
#define USERNAME_LEN 10

struct fils *fils;
int nb_utilisateurs = 0;

utilisateur liste[MAX_USERS];

static int is_user_registered(uint16_t id) {
    for (int i = 0; i < nb_utilisateurs; i++) {
        if (liste[i].id == id) {
            return 1;
        }
    }
    return 0;
}

static void *serve(void *arg) {
    int r;
    uint16_t header, id;
    codereq_t codereq;
    char username[USERNAME_LEN];

    int sock = *(int *)arg;

    char buf[SIZE_MESS * 2];
    memset(buf, 0, SIZE_MESS * 2);

    recv_send_message(sock, buf, SIZE_MESS * 2, RECV);

    memcpy(&header, buf, sizeof(uint16_t));
    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;

    // ENVOIE ENTETE ERREUR SI ID DIFFERENT DE 0 LORSQUE CODEREQ 1
    if (codereq == REQ_INSCRIPTION && id != 0) {
        error_request(sock, codereq, id, ERR_NON_ZERO_ID_WITH_CODE_REQ_ONE);
        return NULL;
    }
    // ENVOIE ENTETE ERREUR SI ID N'EXISTE PAS POUR LES AUTRES REQUETES
    if (codereq != REQ_INSCRIPTION && !is_user_registered(id)) {
        error_request(sock, codereq, id, ERR_ID_DOES_NOT_EXIST);
        return NULL;
    }

    switch (codereq) {
    case REQ_INSCRIPTION:
        r = inscription_request(sock, buf, liste, nb_utilisateurs);
        if (r == 0)
            nb_utilisateurs++;
        break;
    case REQ_POST_BILLET:
        for (int i = 0; i < nb_utilisateurs; i++) {
            if (liste[i].id == id) {
                strcpy(username, liste[i].pseudo);
                break;
            }
        }

        r = post_billet_request(sock, buf, fils, username);
        break;
    case REQ_GET_BILLET:
        r = get_billets_request(sock, buf, fils);
        break;
    case REQ_SUBSCRIBE:
    case REQ_ADD_FILE:
    case REQ_DW_FILE:
        fprintf(stderr, "%s:%d: TODO\n", __FILE__, __LINE__);
        exit(1);
    default:
        error_request(sock, codereq, id, ERR_CODEREQ_UNKNOWN);
        break;
    }

    close(sock);
    free(arg);

    return NULL;
}

static void loop(int sock) {
    struct sockaddr_in6 addrclient = {0};
    socklen_t size = sizeof(addrclient);

    int sock_client;
    while ((sock_client = accept(sock, (struct sockaddr *)&addrclient, &size))) {
        if (sock_client < 0) continue;

        // on crée la variable sur le tas
        int *sock_client_stack = malloc(sizeof(int));
        *sock_client_stack = sock_client;

        pthread_t thread;
        int r = pthread_create(&thread, NULL, serve, sock_client_stack);
        if (r < 0) {
            perror("pthread_create");
            continue;
        }

        char nom_dst[INET6_ADDRSTRLEN + 1] = {0};
        const char *rs = inet_ntop(AF_INET6, &addrclient.sin6_addr, nom_dst, INET6_ADDRSTRLEN);
        if (rs == NULL) {
            strcpy(nom_dst, "<erreur>");
        }

        int port = htons(addrclient.sin6_port);

        printf("CONNEXION CLIENT : %s (port %d)\n", nom_dst, port);
    }
}

static int create_server(int port) {
    int sock = -1;
    int r;

    sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock < 0) goto fail;

    struct sockaddr_in6 address_sock = {0};
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(port);
    address_sock.sin6_addr = in6addr_any;

    int yes = 1;
    r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (r < 0) goto fail;

    r = bind(sock, (struct sockaddr *)&address_sock, sizeof(address_sock));
    if (r < 0) goto fail;

    r = listen(sock, 0);
    if (r < 0) goto fail;

    return sock;

 fail:
    if (sock >= 0) close(sock);
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage : ./serveur <PORT>\n");
        exit(1);
    }

    fils = malloc(sizeof(struct fils));
    fils->nb_fil = 0;

    int port = atoi(argv[1]);
    int sock = create_server(port);

    loop(sock);

    close(sock);

    free(fils);

    return 0;
}
