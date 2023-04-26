#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "../headers/billet.h"

int add_billet(struct fils *fils, uint16_t numfil, uint16_t id, uint8_t lendata, char *data, char *username) {
    int last_billet = fils->list_fil[numfil-1].nb_billet;
    if (last_billet >= 100) {
        return -1;
    }
    
    struct billet new_billet;
    new_billet.idClient = id;
    new_billet.len = lendata;
    strcpy(new_billet.pseudo, username);
    memcpy(new_billet.contenu, data, lendata);

    memcpy(&fils->list_fil[numfil-1].billets[last_billet], &new_billet, sizeof(struct billet));
    fils->list_fil[numfil-1].nb_billet++;

    return 0;
}

int create_fil(struct fils *fils, uint16_t id, uint8_t lendata, char *data, char *username) {
    if (fils->nb_fil >= 100) {
        return -1;
    }

    struct fil new_fil;
    struct billet new_billet;
    new_billet.idClient = id;
    strcpy(new_billet.pseudo, username);
    new_billet.len = lendata;
    memcpy(new_billet.contenu, data, lendata);

    memcpy(&new_fil.billets[0], &new_billet, sizeof(struct billet));
    new_fil.nb_billet = 1;
    strcpy(new_fil.pseudo, username);
    memcpy(&fils->list_fil[fils->nb_fil], &new_fil, sizeof(struct fil));
    fils->nb_fil++;

    return fils->nb_fil;
}

