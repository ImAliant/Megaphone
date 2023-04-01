#ifndef FUNC_CLIENT_H
#define FUNC_CLIENT_H

#include <stdint.h>

// Requete d'erreur
void error_request(uint8_t);
// Suppression des caracteres invisibles
void remove_special_chars(char *);
// Remplissage du pseudo par des #
void completion_pseudo(char *);
// Demande du pseudo
void demande_pseudo(char *);
// Creation de l'entete du serveur
uint16_t create_header(uint8_t);
// Remplissage entete buffer
void header_username_buffer(char *, uint16_t, char *);
//Connexion au serveur
int connexion_server(char *, char *) ;
//Requete d'inscription
void inscription_request(char *, char *);

#endif