#ifndef FUNC_SERVEUR_H
#define FUNC_SERVEUR_H

#include "billet.h"
#include "error.h"
#include "request.h"
#include "users.h"

typedef enum {
    TYPE_ERROR,
    /* nb == 0 */
    TYPE_ALL_BILLETS_OF_ONE_FIL,
    /* f == 0 */
    TYPE_NBILLETS_OF_ALL_FIL,
    /* nb == f == 0 */
    TYPE_ALL_BILLETS_OF_ALL_FILS,
    /* nb != 0 && f != 0 */
    TYPE_NORMAL,
} billet_type_t;

// Requete d'inscription
int inscription_request(int, char *, utilisateur[], int);

// Requete poster un billet
int post_billet_request(int, char *, struct fils *, const char *);

// Requete obtention des billets
int get_billets_request(int, const char *, struct fils *);

// Requete d'erreur
void error_request(int, codereq_t, uint16_t, error_t);

#endif
