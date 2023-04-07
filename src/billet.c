#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "../headers/billet.h"

int create_fil(struct fils *fils, uint16_t id, uint8_t lendata, char *data) {
    if (fils->nb_fil >= 100) {
        return -1;
    }

    struct fil new_fil;
    struct billet new_billet;
    new_billet.idClient = id;
    new_billet.len = lendata;
    memcpy(new_billet.contenu, data, lendata);

    new_fil.billets[0] = new_billet;
    new_fil.nb_billet = 1;
    fils->list_fil[fils->nb_fil] = new_fil;
    fils->nb_fil++;

    return fils->nb_fil;
}

int add_billet(struct fils *fils, uint16_t numfil, uint16_t id, uint8_t lendata, char *data) {
    int last_billet = fils->list_fil[numfil].nb_billet;
    if (last_billet >= 100) {
        return -1;
    }
    
    struct billet new_billet;
    new_billet.idClient = id;
    new_billet.len = lendata;
    memcpy(new_billet.contenu, data, lendata);

    fils->list_fil[numfil].billets[last_billet] = new_billet;
    fils->list_fil[numfil].nb_billet++;

    return 0;
}