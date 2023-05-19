#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/billet.h"

static struct billet create_billet(uint16_t id, uint8_t lendata, const char *data, username_t username) {
    struct billet new_billet;
    new_billet.idClient = id;
    new_billet.len = lendata;
    memcpy(new_billet.pseudo, username, USERNAME_LEN);
    memcpy(new_billet.contenu, data, lendata);
    new_billet.contenu[lendata] = '\0';

    return new_billet;
}

int add_billet(struct fils *fils, uint16_t numfil, uint16_t id, uint8_t lendata,
               const char *data, username_t username) {
    int last_billet = fils->list_fil[numfil - 1].nb_billet;
    if (last_billet >= 100) {
        return -1;
    }

    struct billet new_billet = create_billet(id, lendata, data, username);

    memcpy(&fils->list_fil[numfil - 1].billets[last_billet], &new_billet,
           sizeof(struct billet));
    fils->list_fil[numfil - 1].nb_billet++;

    return 0;
}

int create_fil(struct fils *fils, uint16_t id, uint8_t lendata, const char *data, username_t username) {
    if (fils->nb_fil >= 100) {
        return -1;
    }

    struct billet new_billet = create_billet(id, lendata, data, username);

    struct fil new_fil;
    memcpy(&new_fil.billets[0], &new_billet, sizeof(struct billet));
    new_fil.nb_billet = 1;
    memcpy(new_fil.pseudo, username, USERNAME_LEN);
    memcpy(&fils->list_fil[fils->nb_fil], &new_fil, sizeof(struct fil));
    fils->nb_fil++;

    return fils->nb_fil;
}
