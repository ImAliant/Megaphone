#ifndef BILLET_H
#define BILLET_H

#define SIZE_MESS 200
#define NB_MAX_BILLETS_PAR_FIL 100
#define NB_MAX_FILS 100

#include <stdint.h>

#include "users.h"

// Structure des billets
struct billet {
    uint16_t idClient;
    username_t pseudo;
    char contenu[SIZE_MESS + 1];
    uint8_t len;
};

// Structure des fils
struct fil {
    struct billet billets[NB_MAX_BILLETS_PAR_FIL];
    username_t pseudo;
    int nb_billet;
};

// Liste des fils
struct fils {
    struct fil list_fil[NB_MAX_FILS];
    int nb_fil;
};

// Creation d'un nouveau fil
int create_fil(struct fils *, uint16_t, uint8_t, const char *, username_t);
// Ajout d'un billet dans un fil
int add_billet(struct fils *, uint16_t, uint16_t, uint8_t, const char *, username_t);

#endif
