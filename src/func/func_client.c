#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "billet.h"
#include "error.h"
#include "func/func_client.h"
#include "message.h"
#include "request.h"

#define MAX_VALUE_11BITS_INTEGER ((1 << 11) - 1)
#define BUFLEN 4096

int recv_header(int sock, server_header_t *header) {
    memset(header, 0, sizeof(server_header_t));

    uint16_t codereq_id;
    int r = recv_uint16(sock, &codereq_id);
    if (r < 0) return r;

    header->codereq = codereq_id % (1 << 5);
    header->id = codereq_id >> 5;

    r = recv_uint16(sock, &header->numfil);
    if (r < 0) return r;

    return recv_uint16(sock, &header->nb);
}

int send_header(int sock, client_header_t header) {
    uint16_t buf = 0;

    buf |= header.codereq % (1 << 5);
    buf |= header.id << 5;

    return send_uint16(sock, buf);
}

int error_request(server_header_t header) {
    if (header.codereq == 31) {
        fprintf(stderr, "ERREUR SERVEUR\n");
        return 1;
    }

    if (header.codereq > 0) return 0;

    switch (header.numfil) {
    case ERR_CODEREQ_UNKNOWN:
        printf("ERREUR : REQUETE INCONNUE\n");
        break;
    case ERR_NON_ZERO_ID_WITH_CODE_REQ_ONE:
        printf("ERREUR : ID NON NUL AVEC CODEREQ=1\n");
        break;
    case ERR_ID_DOES_NOT_EXIST:
        printf("ERREUR : ID INEXISTANT DANS LA TABLE\n");
        break;
    case ERR_PSEUDO_ALREADY_USED:
        printf("ERREUR : PSEUDO DEJA UTILISE\n");
        break;
    case ERR_MAX_FILS_REACHED:
        printf("ERREUR : IMPOSSIBLE DE CREER UN NOUVEAU FIL\n");
        break;
    case ERR_MAX_USERS_REACHED:
        printf("ERREUR : IMPOSSIBLE DE CREER UN NOUVEAU UTILISATEUR\n");
        break;
    case ERR_MAX_BILLETS_REACHED:
        printf("ERREUR : IMPOSSIBLE DE CREER UN NOUVEAU BILLET\n");
        break;
    case ERR_NUMFIL:
        printf("ERREUR : NUMERO DE FIL INEXISTANT\n");
        break;
    default:
        fprintf(stderr, "ERREUR : ERREUR INCONNUE\n");
        exit(1);
    }

    return 1;
}

void demande_pseudo(username_t username) {
    bool ok = false;
    char buf[BUFLEN];
    while (!ok) {
        printf("Saisir votre pseudo: ");
        const char *r = fgets(buf, BUFLEN, stdin);
        if (r == NULL) {
            fprintf(stderr, "Erreur: EOF\n");
            exit(1);
        }

        username_error ru = string_to_username(buf, username);
        switch (ru) {
        case USERNAME_OK:
            ok = true;
            break;
        case USERNAME_TOO_LONG:
            printf("Le pseudo ne peut pas depasser 10 caractères\n");
            break;
        case USERNAME_EMPTY:
            break;
        }
    }

    printf("PSEUDO : %s\n", username_to_string(username, buf));
}

uint16_t create_header(uint8_t codereq_client) {
    uint16_t id = 0;
    uint16_t header_client = htons((id << 5) | (codereq_client & 0x1F));

    return header_client;
}

static int get_server_addr(const char *hostname, const char *port, int *sock,
                           struct sockaddr_in6 *addr) {
    struct addrinfo hints, *r, *p;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED;

    ret = getaddrinfo(hostname, port, &hints, &r);
    if (ret != 0 || r == NULL) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }

    p = r;
    while (p != NULL) {
        if ((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) > 0) {
            if (connect(*sock, p->ai_addr, sizeof(struct sockaddr_in6)) == 0) break;
            close(*sock);
        }

        p = p->ai_next;
    }

    if (p == NULL) return -2;

    if (addr != NULL) {
        // on stocke l'adresse de connexion
        memcpy(addr, p->ai_addr, sizeof(struct sockaddr_in6));
    }

    // on libère la mémoire allouée par getaddrinfo
    freeaddrinfo(r);

    return 0;
}

int connexion_server(const char *hostname, const char *port) {
    int sock;
    struct sockaddr_in6 server_addr;

    switch (get_server_addr(hostname, port, &sock, &server_addr)) {
    case 0:
        break;
    case -1:
        fprintf(stderr, "Erreur: hote non trouve.\n");
        exit(1);
    case -2:
        fprintf(stderr, "Erreur: echec de creation de la socket.\n");
        exit(1);
    }

    return sock;
}

int inscription_request(int sock) {
    client_header_t header = {0};
    header.codereq = REQ_INSCRIPTION;
    header.id = 0;

    username_t username;
    demande_pseudo(username);

    // ENVOI ENTETE + PSEUDO
    int r = send_header(sock, header);
    if (r < 0) return r;
    r = send_raw(sock, username, USERNAME_LEN);
    if (r < 0) return r;

    // RECEPTION ENTETE + ID
    server_header_t response_header;
    r = recv_header(sock, &response_header);
    if (r < 0) return r;

    printf("codereq: %d, id: %d\n", response_header.codereq, response_header.id);

    if (error_request(response_header)) {
        close(sock);
        exit(5);
    }

    printf("VOICI VOTRE ID : %d\n", response_header.id);

    close(sock);

    return 0;
}

int post_billet_request(int sock) {
    printf("IDENTIFIANT ET NUM FIL (0 pour en créer un nouveau) : ");
    fflush(stdout);

    uint16_t id, numfil;
    int r = scanf("%hu %hu", &id, &numfil);

    if (r == EOF) {
        perror("Erreur: ");
    } else if (r != 2) {
        fprintf(stderr, "Erreur : EOF\n");
    }

    if (id > MAX_VALUE_11BITS_INTEGER) {
        fprintf(stderr, "Erreur : Cet identifiant ne peux pas exister.\n");
        exit(1);
    }

    client_header_t header = {0};
    header.codereq = REQ_POST_BILLET;
    header.id = id;

    printf("ENTREZ VOTRE MESSAGE : ");
    getchar();
    fflush(stdout);

    char data[SIZE_MESS] = {0};
    const char *r2 = fgets(data, SIZE_MESS, stdin);
    if (r2 == NULL) {
        fprintf(stderr, "Erreur : EOF\n");
        exit(1);
    }

    uint8_t lendata = strlen(data);
    if (data[lendata - 1] == '\n') {
        data[lendata - 1] = '\0';
        lendata--;
    }
    printf("len: %d\n", lendata);

    // ENVOI DE LA REQUETE
    r = send_header(sock, header);
    if (r < 0) return r;
    r = send_uint16(sock, numfil);
    if (r < 0) return r;
    r = send_uint16(sock, 0); // NB
    if (r < 0) return r;
    r = send_uint8(sock, lendata);
    if (r < 0) return r;
    r = send_raw(sock, data, lendata);
    if (r < 0) return r;

    // RECEPTION DE LA REPONSE
    server_header_t response_header;
    r = recv_header(sock, &response_header);
    if (r < 0) return r;

    if (error_request(response_header)) {
        close(sock);
        exit(5);
    }

    // DECODAGE DE LA REPONSE
    // AFFICHAGE DE LA REPONSE
    printf("REPONSE : CODEREQ %hd, ID %hd, NUMFIL %hd\n", response_header.codereq,
           response_header.id, response_header.numfil);

    close(sock);

    return 0;
}

int get_billets_request(int sock) {
    uint16_t id, numfil, nb;

    printf("IDENTIFIANT, NUMERO DU FIL ET NB DE BILLETS : ");
    fflush(stdout);

    int r = scanf("%hu %hu %hu", &id, &numfil, &nb);
    if (r < 3) {
        fprintf(stderr, "Erreur : EOF\n");
        return -1;
    }

    if (id > MAX_VALUE_11BITS_INTEGER) {
        fprintf(stderr, "Erreur : Cet identifiant ne peux pas exister.\n");
        return -1;
    }

    // CONTRUCTION ENTETE
    client_header_t header = {0};
    header.codereq = REQ_GET_BILLET;
    header.id = id;

    // ENVOI DE LA REQUETE
    r = send_header(sock, header);
    if (r < 0) return r;

    r = send_uint16(sock, numfil);
    if (r < 0) return r;

    r = send_uint16(sock, nb);
    if (r < 0) return r;

    r = send_uint16(sock, 0); // lendata
    if (r < 0) return r;

    // RECEPTION DE LA REPONSE
    server_header_t response_header;
    recv_header(sock, &response_header);

    if (error_request(response_header)) {
        close(sock);
        exit(5);
    }

    // AFFICHAGE DE LA REPONSE
    printf("REPONSE : CODEREQ %hd, ID %hd, NUMFIL %hd, NB %hd\n",
           response_header.codereq, response_header.id, response_header.numfil,
           response_header.nb);

    // RECEPTION DES BILLETS
    for (int i = 0; i < nb; i++) {
        uint16_t numfil;
        r = recv_uint16(sock, &numfil);
        if (r < 0) return r;

        username_t pseudo_fil;
        r = recv_raw(sock, pseudo_fil, USERNAME_LEN);
        if (r < 0) return r;

        username_t pseudo_billet;
        r = recv_raw(sock, pseudo_billet, USERNAME_LEN);
        if (r < 0) return r;

        uint8_t lendata;
        r = recv_uint8(sock, &lendata);
        if (r < 0) return r;

        char data[lendata + 1];
        memset(data, 0, lendata + 1);
        r = recv_raw(sock, data, lendata);
        if (r < 0) return r;

        // AFFICHAGE DU BILLET
        char buf_pseudo_fil[USERNAME_LEN + 1];
        username_to_string(pseudo_fil, buf_pseudo_fil);
        char buf_pseudo_billet[USERNAME_LEN + 1];
        username_to_string(pseudo_billet, buf_pseudo_billet);
        printf("BILLET %d : NUMFIL %hd, ORIGINE %s, PSEUDO %s, DATALEN %d, DATA %s\n",
               i + 1, numfil, buf_pseudo_fil, buf_pseudo_billet, lendata, data);
    }

    return 0;
}
