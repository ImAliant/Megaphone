#include <arpa/inet.h>
#include <ctype.h>
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
#define MAX_USERS 2047
#define ID_BITS 11

int recv_header(int sock, client_header_t *header) {
    uint16_t buf = 0;

    int r = recv_uint16(sock, &buf);
    if (r < 0) return r;

    memset(header, 0, sizeof(client_header_t));

    header->codereq = buf >> 11;
    header->id = buf % (1 << 11);

    return 0;
}

int send_header(int sock, server_header_t header) {
    uint16_t codereq_id = 0;
    codereq_id |= header.codereq << 11;
    codereq_id |= header.id % (1 << 12);

    int r = send_uint16(sock, codereq_id);
    if (r < 0) return r;
    r = send_uint16(sock, header.numfil);
    if (r < 0) return r;
    r = send_uint16(sock, header.nb);

    return r;
}

static uint16_t generate_user_id(username_t username) {
    // HASH
    uint16_t id = 0;
    unsigned long hash = 5381;
    int c;
    while ((c = *username++)) {
        hash = ((hash << 5) + hash) + c;
    }

    for (int i = 0; i < ID_BITS; i++) {
        int bit = (hash >> i) & 1;
        id |= bit << i;
    }

    return id;
}

static void create_new_user(username_t username, uint16_t user_id, utilisateur liste[], int *nb_utilisateurs) {
    utilisateur user;
    memcpy(user.pseudo, username, USERNAME_LEN);
    user.id = user_id;

    liste[*nb_utilisateurs] = user;
    (*nb_utilisateurs)++;
}

static int is_pseudo_used(const username_t username, const utilisateur liste[], int nb_utilisateurs) {
    for (int i = 0; i < nb_utilisateurs; i++) {
        if (strcmp(liste[i].pseudo, username) == 0) {
            return 1;
        }
    }
    return 0;
}

int inscription_request(int sock, client_header_t header, utilisateur liste[], int *nb_utilisateurs) {
    username_t username;

    int r = recv_raw(sock, username, USERNAME_LEN);
    if (r < 0) return r;

    // TEST SI LE NOMBRE MAX D'UTILISATEURS EST ATTEINT
    if (*nb_utilisateurs >= MAX_USERS) {
        error_request(sock, header.codereq, header.id, ERR_MAX_USERS_REACHED);
        return -1;
    }

    // TEST SI LE PSEUDONYME EST DEJA UTILISE
    if (is_pseudo_used(username, liste, *nb_utilisateurs)) {
        error_request(sock, header.codereq, header.id, ERR_PSEUDO_ALREADY_USED);
        return -1;
    }

    server_header_t res_header = {0};
    res_header.codereq = REQ_INSCRIPTION;
    res_header.id = generate_user_id(username);

    create_new_user(username, res_header.id, liste, nb_utilisateurs);

    for (int i = 0; i < *nb_utilisateurs; i++) {
        char usrname_buf[USERNAME_LEN + 1];
        username_to_string(liste[i].pseudo, usrname_buf);
        printf("%d: %s\n", liste[i].id, usrname_buf);

    }

    return send_header(sock, res_header);
}

int post_billet_request(int sock, client_header_t header, struct fils *fils, username_t username) {
    // TRADUCTION DU MESSAGE DU CLIENT
    // RECUPERATION DE L'ENTETE
    uint16_t numfil;
    int r = recv_uint16(sock, &numfil);
    if (r < 0) return r;

    uint16_t nb;
    r = recv_uint16(sock, &nb);
    if (r < 0) return r;

    uint8_t lendata = 0;
    r = recv_uint8(sock, &lendata);
    if (r < 0) return r;

    char data[lendata];
    r = recv_raw(sock, data, lendata);

    // AFFICHAGE DU MESSAGE DU CLIENT
    printf("CODEREQ %hd, ID %hd, NUMFIL %hd, NB %hd, LENDATA %hd, DATA %.*s\n",
           header.codereq, header.id, numfil, nb, lendata, lendata, data);

    // SI LE NUMERO DE FIL EST 0, CREER UN NOUVEAU FIL
    if (numfil == 0) {
        int newfil = create_fil(fils, header.id, lendata, data, username);

        if (newfil < 0) {
            error_request(sock, header.codereq, header.id, ERR_MAX_FILS_REACHED);
            return 1;
        }
    } else {
        // SINON, AJOUTER LE BILLET AU FIL

        // TEST SI LE NUMERO DE FIL EST VALIDE
        if (numfil > fils->nb_fil) {
            error_request(sock, header.codereq, header.id, ERR_NUMFIL);
            return 1;
        }

        if (add_billet(fils, numfil, header.id, lendata, data, username) < 0) {
            error_request(sock, header.codereq, header.id, ERR_MAX_BILLETS_REACHED);
            return 1;
        }
    }

    server_header_t res_header = {0};
    // REPONSE AU CLIENT
    res_header.codereq = REQ_POST_BILLET;
    res_header.id = header.id;
    res_header.numfil = numfil;
    res_header.nb = 0;

    return send_header(sock, res_header);
}

int error_request(int sock, codereq_t codereq_client, uint16_t id, error_t err) {
    server_header_t res_header = {0};
    switch (err) {
    case ERR_CODEREQ_UNKNOWN:
        printf("CODEREQ INCONNU : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_NON_ZERO_ID_WITH_CODE_REQ_ONE:
        printf("ID NON NUL AVEC CODEREQ 1, ID %d\n", id);
        break;
    case ERR_ID_DOES_NOT_EXIST:
        printf("ID INCONNU : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_PSEUDO_ALREADY_USED:
        printf("PSEUDONYME DEJA UTILISE : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_MAX_FILS_REACHED:
        printf("IMPOSSIBLE DE CREER UN NOUVEAU FIL : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_MAX_BILLETS_REACHED:
        printf("IMPOSSIBLE DE CREER UN NOUVEAU BILLET : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_MAX_USERS_REACHED:
        printf("IMPOSSIBLE DE CREER UN NOUVEAU UTILISATEUR : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_NUMFIL:
        printf("NUMERO DE FIL INCONNU : <%d>, ID %d\n", codereq_client, id);
        break;
    default:
        fprintf(stderr, "code erreur inconnu\n");
        exit(1);
    }

    res_header.codereq = 0;
    res_header.id = id;
    codereq_client = REQ_SERVER_ERROR;

    return send_header(sock, res_header);
}

static int get_nb_billets(struct fils *fils, uint16_t numfil, uint16_t nb,
                          billet_type_t type) {
    int nb_billets = 0;
    // TOUS LES BILLETS DANS UN FIL
    if (type == TYPE_ALL_BILLETS_OF_ONE_FIL) {
        nb_billets = fils->list_fil[numfil - 1].nb_billet;
    }
        // N BILLETS DANS CHAQUE FILS
    else if (type == TYPE_NBILLETS_OF_ALL_FIL) {
        for (int i = 0; i < fils->nb_fil; i++) {
            nb_billets +=
                (fils->list_fil[i].nb_billet > nb) ? nb : fils->list_fil[i].nb_billet;
        }
    }
        // TOUT LES BILLETS DE CHAQUE FILS
    else if (type == TYPE_ALL_BILLETS_OF_ALL_FILS) {
        for (int i = 0; i < fils->nb_fil; i++) {
            nb_billets += fils->list_fil[i].nb_billet;
        }
    } else if (type == TYPE_NORMAL) {
        nb_billets = (fils->list_fil[numfil - 1].nb_billet > nb)
        ? nb
        : fils->list_fil[numfil - 1].nb_billet;
    }

    return nb_billets;
}

static int send_billet(int sock, struct fils *fils, uint16_t numfil, int pos_billet) {
    struct fil fil = fils->list_fil[numfil];
    struct billet billet = fil.billets[pos_billet];

    char buf_pseudo_fil[USERNAME_LEN + 1]; username_to_string(fil.pseudo, buf_pseudo_fil);
    char buf_pseudo_billet[USERNAME_LEN + 1]; username_to_string(billet.pseudo, buf_pseudo_billet);
    printf("BILLET %d DU FIL %d : PSEUDO FIL %s, PSEUDO BILLET %s, LEN DATA %d, DATA %s\n",
           pos_billet, numfil, buf_pseudo_fil, buf_pseudo_billet, billet.len, billet.contenu);
    printf("ENVOI DU BILLET %d DU FIL %d\n", pos_billet, numfil);

    int r = send_uint16(sock, numfil);
    if (r < 0) return r;

    r = send_raw(sock, fil.pseudo, USERNAME_LEN);
    if (r < 0) return r;

    r = send_raw(sock, billet.pseudo, USERNAME_LEN);
    if (r < 0) return r;

    r = send_uint8(sock, billet.len);
    if (r < 0) return r;

    return send_raw(sock, billet.contenu, billet.len);
}

static int send_type_all_billets_of_one_fil(int sock, struct fils *fils, uint16_t numfil) {
    int nombre_billets = fils->list_fil[numfil - 1].nb_billet;

    for (int i = nombre_billets - 1; i >= 0; i--) {
        int r = send_billet(sock, fils, numfil - 1, i);
        if (r < 0) return r;
    }

    return 0;
}

static int send_type_nbillets_of_all_fil(int sock, struct fils *fils, int n) {
    int nombre_fils = fils->nb_fil;

    for (int i = 0; i < nombre_fils; i++) {
        int nombre_billets_temp = fils->list_fil[i].nb_billet;

        for (int j = nombre_billets_temp - 1; j >= nombre_billets_temp - n && j >= 0; j--) {
            int r = send_billet(sock, fils, i, j);
            if (r < 0) return r;
        }
    }

    return 0;
}

static int send_type_all_billets_of_all_fil(int sock, struct fils *fils) {
    int nombre_fils = fils->nb_fil;

    for (int i = nombre_fils - 1; i >= 0; i--) {
        int nombre_billets = fils->list_fil[i].nb_billet;

        for (int j = nombre_billets - 1; j >= 0; j--) {
            int r = send_billet(sock, fils, i, j);
            if (r < 0) return r;
        }
    }

    return 0;
}

static int send_type_normal(int sock, struct fils *fils, uint16_t numfil, int n) {
    int nombre_billets = fils->list_fil[numfil - 1].nb_billet;

    for (int j = nombre_billets - 1; j >= nombre_billets - n && j >= 0; j--) {
        int r = send_billet(sock, fils, numfil - 1, j);
        if (r < 0) return r;
    }

    return 0;
}

static int send_billets(int sock, struct fils *fils, uint16_t numfil, int n, billet_type_t type) {
    switch (type) {
    case TYPE_ALL_BILLETS_OF_ONE_FIL:
        return send_type_all_billets_of_one_fil(sock, fils, numfil);
    case TYPE_NBILLETS_OF_ALL_FIL:
        return send_type_nbillets_of_all_fil(sock, fils, numfil);
    case TYPE_ALL_BILLETS_OF_ALL_FILS:
        return send_type_all_billets_of_all_fil(sock, fils);
    case TYPE_NORMAL:
        return send_type_normal(sock, fils, numfil, n);
    default: return -1;
    }
}

static billet_type_t find_case_type(uint16_t numfil, uint16_t nb) {
    billet_type_t type;
    if (numfil == 0 && nb == 0) {
        type = TYPE_ALL_BILLETS_OF_ALL_FILS;
    } else if (numfil == 0) {
        type = TYPE_NBILLETS_OF_ALL_FIL;
    } else if (nb == 0) {
        type = TYPE_ALL_BILLETS_OF_ONE_FIL;
    } else {
        type = TYPE_NORMAL;
    }

    return type;
}

static int send_num_billets_to_client(int sock, client_header_t header, uint16_t numfil, int nb_billets) {
    server_header_t res_header;
    res_header.codereq = header.codereq;
    res_header.id = header.id;
    res_header.numfil = numfil;
    res_header.nb = nb_billets;

    return send_header(sock, res_header);
}

int get_billets_request(int sock, client_header_t header, struct fils *fils) {
    uint16_t numfil;
    int r = recv_uint16(sock, &numfil);
    if (r < 0) return r;

    uint16_t nb;
    r = recv_uint16(sock, &nb);
    if (r < 0) return r;

    uint16_t lendata;
    r = recv_uint16(sock, &lendata);
    if (r < 0) return r;

    if (lendata != 0) {
        error_request(sock, header.codereq, header.id, ERR_TODO);
        return -1;
    }

    printf("CODEREQ %hd, ID %hd, NUMFIL %hd, NB %hd\n", header.codereq, header.id, numfil, nb);

    // TEST SI LE NUMERO DE FIL EST VALIDE
    if (numfil > fils->nb_fil) {
        error_request(sock, header.codereq, header.id, ERR_NUMFIL);
        return 1;
    }

    billet_type_t type = find_case_type(numfil, nb);
    if (type == TYPE_ERROR) {
        return 1;
    }

    int nb_billets = get_nb_billets(fils, numfil, nb, type);

    // ENVOIE LE NOMBRE DE BILLETS QUE VA RECEVOIR LE CLIENT
    r = send_num_billets_to_client(sock, header, numfil, nb_billets);
    if (r < 0) return r;

    // ON ENVOIE LES BILLETS AU CLIENT
    return send_billets(sock, fils, numfil, nb, type);
}
