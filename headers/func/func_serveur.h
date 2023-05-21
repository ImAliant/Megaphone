#ifndef FUNC_SERVEUR_H
#define FUNC_SERVEUR_H

#include "billet.h"
#include "error.h"
#include "message.h"
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
int inscription_request(int, client_header_t, utilisateur[], int *);

// Requete poster un billet
int post_billet_request(int, client_header_t, struct fils *, username_t);

// Requete obtention des billets
int get_billets_request(int, client_header_t, struct fils *);

// Requete d'erreur
int error_request(int, codereq_t, uint16_t, error_t);

// reçoit un header du client
int recv_header(int sock, client_header_t *header);

// envoie un header au client
int send_header(int sock, server_header_t header);

#endif
