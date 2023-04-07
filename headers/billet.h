#ifndef BILLET_H
#define BILLET_H
#define SIZE_MESS 200

// Structure des billets
struct billet {
    uint16_t idClient;
    char contenu[SIZE_MESS+1];
    uint8_t len;
};

// Structure des fils
struct fil {
    struct billet billets[100];
    int nb_billet;
};

// Liste des fils
struct fils {
    struct fil list_fil[100];
    int nb_fil;
};

// Creation d'un nouveau fil
int create_fil(struct fils *, uint16_t, uint8_t, char *);
// Ajout d'un billet dans un fil
int add_billet(struct fils *, uint16_t, uint16_t, uint8_t, char *);

#endif 