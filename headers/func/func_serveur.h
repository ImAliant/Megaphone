#ifndef FUNC_SERVEUR_H
#define FUNC_SERVEUR_H

#include <ctype.h>
#include "../users.h"

// Retire les caracteres speciaux d'un pseudo
void remove_special_chars(char*);

// Demande si l'utilisateur veut se connecter ou s'inscrire
int demande_client_insc_conn(int);

// Demande le pseudo de l'utilisateur
void demande_client_pseudo(int, char*, char*);

// Demande l'id de l'utilisateur
void demande_client_id(int, char*);

// Complète le pseudo de l'utilisateur
void completion_pseudo(char*);

// envoi l'id au client
void envoi_id_client(int, int);

// Cherche le pseudo et l'id dans la liste des utilisateurs
void find_pseudo_id_in_list(char *, char *, int, int, utilisateur[]);

#endif