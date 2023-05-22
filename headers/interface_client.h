#ifndef INTERFACE_CLIENT_H
#define INTERFACE_CLIENT_H

#include <stdbool.h>
#include <stdlib.h>

// Pose une question a l'utilisateur
int ask(const char *, char *, size_t);

// Renvoie le charactere unique contenu dans la chaine, < 0 sinon
char getCharacter(const char *);

#endif
