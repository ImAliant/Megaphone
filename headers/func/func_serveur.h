#ifndef FUNC_SERVEUR_H
#define FUNC_SERVEUR_H

#include "../users.h"

// Genere l'identifiant de l'utilsateur
int generate_user_id(char *);

// Reception de l'entete du client
int recv_header_client(int, char *);

// Requete d'inscription
int inscription_request(int, char *, utilisateur[], int);

// Requete poster un billet
int post_billet_request(int, char *, struct fils*);

// Requete d'erreur
void error_request(int, uint8_t, uint16_t, int);

#endif