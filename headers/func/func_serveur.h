#ifndef FUNC_SERVEUR_H
#define FUNC_SERVEUR_H

#include "../users.h"

// Genere l'identifiant de l'utilsateur
int generate_user_id();

// Reception de l'entete du client
uint16_t recv_header_client(int, char *, uint16_t);

// Requete d'inscription
int inscription_request(int, char *, utilisateur[], int);

// Requete d'erreur
void error_request(int, uint8_t, uint16_t);

#endif