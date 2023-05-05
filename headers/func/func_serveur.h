#ifndef FUNC_SERVEUR_H
#define FUNC_SERVEUR_H

#include "../users.h"

// Requete d'inscription
int inscription_request(int, char *, utilisateur[], int);

// Requete poster un billet
int post_billet_request(int, char *, struct fils*, char *);

// Requete obtention des billets
int get_billets_request(int, char *, struct fils*);

// Requete d'erreur
void error_request(int, uint8_t, uint16_t, int);

// Requete abonnement fil
int subscribe_request(int, char *, struct fils*);

#endif