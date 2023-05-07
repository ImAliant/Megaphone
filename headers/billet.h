#ifndef BILLET_H
#define BILLET_H

#define SIZE_MESS 200
#define MAX_USERNAME_LEN 10
#define NB_MAX_BILLETS_PAR_FIL 100
#define NB_MAX_FILS 100

#include <stdint.h>

// Structure des billets
struct billet {
    uint16_t idClient;
    char pseudo[MAX_USERNAME_LEN + 1];
    char contenu[SIZE_MESS + 1];
    uint8_t len;
};

// Structure des fils
struct fil {
    struct billet billets[NB_MAX_BILLETS_PAR_FIL];
    char pseudo[MAX_USERNAME_LEN + 1];
    int nb_billet;
};

// Liste des fils
struct fils {
    struct fil list_fil[NB_MAX_FILS];
    int nb_fil;
};

// Creation d'un nouveau fil
int create_fil(struct fils *, uint16_t, uint8_t, char *, char *);
// Ajout d'un billet dans un fil
int add_billet(struct fils *, uint16_t, uint16_t, uint8_t, char *, char *);

#endif
