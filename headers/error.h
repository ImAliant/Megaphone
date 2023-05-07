#ifndef ERROR_H_
#define ERROR_H_

// CODEREQ inconnu
#define ERR_CODEREQ_UNKNOWN 1
// ID non nul avec CODEREQ=1
#define ERR_NON_ZERO_ID_WITH_CODE_REQ_ONE 2
// ID inexistant dans la table
#define ERR_ID_DOES_NOT_EXIST 3
// PSEUDO déjà utilisé
#define ERR_PSEUDO_ALREADY_USED 4
// Impossible d'ajouter un fil car la table est pleine
#define ERR_MAX_FILS_REACHED 5
// Impossible d'ajouter un utilisateur car la table est pleine
#define ERR_MAX_USERS_REACHED 6
// Impossible d'ajouter un billet car la table est pleine
#define ERR_MAX_BILLETS_REACHED 7
// Numéro de fil inexistant
#define ERR_NUMFIL 8
// Type pour affichage billets
#define ERR_NON_TYPE 9

#endif
