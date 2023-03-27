#ifndef USER_H
#define USER_H

#include <stdint.h>

// Structure d'un utilisateur
typedef struct {
    char *pseudo;
    uint16_t id;
} utilisateur;

#endif