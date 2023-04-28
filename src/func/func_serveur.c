#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "../../headers/socket.h"
#include "../../headers/users.h"
#include "../../headers/billet.h"
#include "../../headers/error.h"
#include "../../headers/func/func_serveur.h"
#include "../../headers/request.h"
#include "../../headers/message.h"

#define SIZE_MESS 200
#define MAX_USERS 100
#define MAX_USERNAME_LEN 10
#define ID_BITS 11

/* Si nb égale à 0 */
#define TYPE_BILLETS_FIL 1
/* Si f égale à 0 */
#define TYPE_BILLETS_OF_ALL_FIL 2
/* Si nb et f égales à 0 */
#define TYPE_ALL_BILLETS 3
/* Si nb et f sont différents de 0 */
#define TYPE_NORMAL 4

uint16_t generate_user_id(const char *username) {
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

void create_new_user(const char *username, uint16_t user_id, utilisateur liste[], int nb_utilisateurs) {
    utilisateur user;
    user.pseudo = malloc(sizeof(char*)*MAX_USERS);
    strcpy(user.pseudo, username);
    user.id = user_id;

    liste[nb_utilisateurs] = user;
    nb_utilisateurs++;
}

int is_pseudo_used(const char *username, const utilisateur liste[], int nb_utilisateurs) {
    for(int i = 0; i < nb_utilisateurs; i++) {
        if(strcmp(liste[i].pseudo, username) == 0) {
            return 1;
        }
    }
    return 0;
}

int inscription_request(int sock_client, char *buf, utilisateur liste[], int nb_utilisateurs) {
    uint16_t header;
    uint16_t id;
    uint8_t codereq;
    char username[MAX_USERNAME_LEN+1];

    memcpy(&header, buf, sizeof(uint16_t));
    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;
    memcpy(username, buf+sizeof(uint16_t), MAX_USERNAME_LEN);
    username[MAX_USERNAME_LEN] = '\0';

    // TEST SI LE NOMBRE MAX D'UTILISATEURS EST ATTEINT
    if (nb_utilisateurs >= MAX_USERS) {
        error_request(sock_client, codereq, id, ERR_MAX_USERS_REACHED);
        return 1;
    }

    // TEST SI LE PSEUDONYME EST DEJA UTILISE
    if(is_pseudo_used(username, liste, nb_utilisateurs)) {
        error_request(sock_client, codereq, id, ERR_PSEUDO_ALREADY_USED);
        return 1;
    }

    // AFFICHAGE DU MESSAGE DU CLIENT
    printf("CODEREQ %hd, ID %hd, PSEUDONYME %s\n", codereq, id, username);

    id = generate_user_id(username);

    create_new_user(username, id, liste, nb_utilisateurs);

    codereq = REQ_INSCRIPTION;

    header = htons((id << 5) | (codereq & 0x1F));
                
    memset(buf, 0, SIZE_MESS);
    memcpy(buf, &header, sizeof(header));

    recv_send_message(sock_client, buf, SIZE_MESS, SEND);

    return 0;
}

int post_billet_request(int sock_client, char *buf, struct fils *fils, char *username) {
    // TRADUCTION DU MESSAGE DU CLIENT
    uint16_t header, id, numfil, nb;
    uint8_t codereq, lendata;
    char data[SIZE_MESS+1];
    memset(data, 0, SIZE_MESS);

    // RECUPERATION DE L'ENTETE
    memcpy(&header, buf, sizeof(uint16_t));
    memcpy(&numfil, buf+sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&nb, buf+sizeof(uint16_t)*2, sizeof(uint16_t));
    memcpy(&lendata, buf+sizeof(uint16_t)*3, sizeof(uint8_t));
    memcpy(data, buf+sizeof(uint16_t)*3+sizeof(uint8_t), lendata);

    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;
    numfil = ntohs(numfil);
    nb = ntohs(nb);

    // AFFICHAGE DU MESSAGE DU CLIENT
    printf("CODEREQ %hd, ID %hd, NUMFIL %hd, NB %hd, LENDATA %hd, DATA %s\n", codereq, id, numfil, nb, lendata, data);

    // SI LE NUMERO DE FIL EST 0, CREER UN NOUVEAU FIL
    if (numfil == 0) {
        numfil = create_fil(fils, id, lendata, data, username);

        if (numfil == -1) {
            error_request(sock_client, codereq, id, ERR_MAX_FILS_REACHED);
            return 1;
        }
    }

    // SINON, AJOUTER LE BILLET AU FIL
    else {
        // TEST SI LE NUMERO DE FIL EST VALIDE
        if (numfil > fils->nb_fil) {
            error_request(sock_client, codereq, id, ERR_NUMFIL);
            return 1;
        }

        if (add_billet(fils, numfil, id, lendata, data, username) == -1) {
            error_request(sock_client, codereq, id, ERR_MAX_BILLETS_REACHED);
            return 1;
        }
    }

    // REPONSE AU CLIENT
    codereq = REQ_POST_BILLET;
    header = htons((id << 5) | (codereq & 0x1F));
    numfil = htons(numfil);
    nb = htons(0);

    memset(buf, 0, SIZE_MESS*2);
    memcpy(buf, &header, sizeof(uint16_t));
    memcpy(buf+sizeof(uint16_t), &numfil, sizeof(uint16_t));
    memcpy(buf+sizeof(uint16_t)*2, &nb, sizeof(uint16_t));

    recv_send_message(sock_client, buf, sizeof(uint16_t)*3, SEND);

    return 0;
}

void error_request(int sock_client, uint8_t codereq_client, uint16_t id, int err) {
    uint16_t header_serv;
    char buf[SIZE_MESS];

    switch (err)
    {
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
        perror("code erreur inconnu");
        exit(EXIT_FAILURE);
    }
    
    codereq_client = 0;
    header_serv = htons((id << 5) | (codereq_client & 0x1F));
    
    memset(buf, 0, SIZE_MESS);
    memcpy(buf, &header_serv, sizeof(uint16_t));
    memcpy(buf+sizeof(uint16_t), &err, sizeof(int));
    
    recv_send_message(sock_client, buf, SIZE_MESS, SEND);
}

int get_nb_billets(struct fils *fils, uint16_t numfil, uint16_t nb, int type) {
    int nb_billets = 0;
    // TOUS LES BILLETS DANS UN FIL
    if (type == TYPE_BILLETS_FIL) {
        nb_billets = fils->list_fil[numfil-1].nb_billet;
    }
    // N BILLETS DANS CHAQUE FILS
    else if (type == TYPE_BILLETS_OF_ALL_FIL) {
        for (int i = 0; i < fils->nb_fil; i++) {
            nb_billets += (fils->list_fil[i].nb_billet > nb) ? nb : fils->list_fil[i].nb_billet;
        }
    }
    // TOUT LES BILLETS DE CHAQUE FILS
    else if (type == TYPE_ALL_BILLETS) {
        for (int i = 0; i < fils->nb_fil; i++) {
            nb_billets += fils->list_fil[i].nb_billet;
        }
    }
    else if (type == TYPE_NORMAL) {
        nb_billets = (fils->list_fil[numfil-1].nb_billet > nb) ? nb : fils->list_fil[numfil-1].nb_billet;
    }

    return nb_billets;
}

int send_billet(int sock, struct fils *fils, uint16_t numfil, int pos_billet) {
    uint8_t lendata;
    char pseudo_fil[MAX_USERNAME_LEN+1];
    char pseudo_billet[MAX_USERNAME_LEN+1];
    char data[SIZE_MESS+1];

    memset(pseudo_fil, 0, MAX_USERNAME_LEN+1);
    memset(pseudo_billet, 0, MAX_USERNAME_LEN+1);
    memset(data, 0, SIZE_MESS+1);

    size_t sizebillet = sizeof(uint16_t)+(MAX_USERNAME_LEN+1)*2+sizeof(uint8_t)+(SIZE_MESS+1);
    char *billet = malloc(sizebillet);
    if (billet == NULL) {
        perror("malloc");
        return 1;
    }

    memcpy(pseudo_fil, fils->list_fil[numfil].pseudo, MAX_USERNAME_LEN+1);
    memcpy(pseudo_billet, fils->list_fil[numfil].billets[pos_billet].pseudo, MAX_USERNAME_LEN+1);
    memcpy(data, fils->list_fil[numfil].billets[pos_billet].contenu, SIZE_MESS+1);
    lendata = fils->list_fil[numfil].billets[pos_billet].len;

    numfil += 1;
    pos_billet += 1;

    printf("BILLET %d DU FIL %d : PSEUDO FIL %s, PSEUDO BILLET %s, LEN DATA %d, DATA %s\n", pos_billet, numfil, pseudo_fil, pseudo_billet, lendata, data);
    printf("ENVOI DU BILLET %d DU FIL %d\n", pos_billet, numfil);

    numfil = htons(numfil);
    
    char *ptr = billet;
    memcpy(ptr, &numfil, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    memcpy(ptr, pseudo_fil, strlen(pseudo_fil)+1);
    ptr += strlen(pseudo_fil)+1;
    memcpy(ptr, pseudo_billet, strlen(pseudo_billet)+1);
    ptr += strlen(pseudo_billet)+1;
    memcpy(ptr, &lendata, sizeof(uint8_t));
    ptr += sizeof(uint8_t);
    memcpy(ptr, data, strlen(data) + 1);

    recv_send_message(sock, billet, sizebillet, SEND);

    return 0;
}

int send_billets(int sock, struct fils *fils, uint16_t numfil, uint16_t nb, int type) {
    if (type == TYPE_BILLETS_FIL) {
        // TODO
    }
    else if (type == TYPE_BILLETS_OF_ALL_FIL) {
        // TODO
    }
    else if (type == TYPE_ALL_BILLETS) {
        int nombre_fils = fils->nb_fil;
        for (int i = 0; i < nombre_fils; i++) {
            int nombre_billets = fils->list_fil[i].nb_billet;
            for (int j = 0; j < nombre_billets; j++) {
                send_billet(sock, fils, i, j);
            }
        }
    }
    else if (type == TYPE_NORMAL) {
        // TODO
    }

    return 0;
}

int get_billets_request(int sock_client, char *buf, struct fils *fils) {
    uint16_t header, codereq, id, numfil, nb;

    char *ptr = buf;
    memcpy(&header, ptr, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    memcpy(&numfil, ptr, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    memcpy(&nb, ptr, sizeof(uint16_t));

    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;
    numfil = ntohs(numfil);
    nb = ntohs(nb);

    printf("CODEREQ %hd, ID %hd, NUMFIL %hd, NB %hd\n", codereq, id, numfil, nb);

    // TEST SI LE NUMERO DE FIL EST VALIDE
    if (numfil > fils->nb_fil) {
        error_request(sock_client, codereq, id, ERR_NUMFIL);
        return 1;
    }

    int type = 0;
    if (numfil == 0 && nb == 0) {
        type = TYPE_ALL_BILLETS;
    }
    else if (numfil == 0) {
        type = TYPE_BILLETS_OF_ALL_FIL;
    }
    else if (nb == 0) {
        type = TYPE_BILLETS_FIL;
    }
    else if (nb != 0 && numfil != 0){
        type = TYPE_NORMAL;
    }
    else {
        error_request(sock_client, codereq, id, ERR_NON_TYPE);
        return 1;
    }

    int nb_billets = get_nb_billets(fils, numfil, nb, type);

    // ENVOIE LE NOMBRE DE BILLETS QUE VA RECEVOIR LE CLIENT
    codereq = REQ_GET_BILLET;
    header = htons((id << 5) | (codereq & 0x1F));
    numfil = htons(numfil);
    nb = htons(nb_billets);

    size_t sizebuf = sizeof(uint16_t)*3;
    char buffer[sizebuf];
    memset(buffer, 0, sizebuf);

    memcpy(buffer, &header, sizeof(uint16_t));
    memcpy(buffer+sizeof(uint16_t), &numfil, sizeof(uint16_t));
    memcpy(buffer+sizeof(uint16_t)*2, &nb, sizeof(uint16_t));

    recv_send_message(sock_client, buffer, sizebuf, SEND);

    // ON ENVOIE LES BILLETS AU CLIENT
    send_billets(sock_client, fils, numfil, nb, type);

    return 0;
}