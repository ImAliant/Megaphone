#ifndef FUNC_CLIENT_H
#define FUNC_CLIENT_H

// Requete d'erreur
void error_request(uint8_t codereq_serv);
// Demande du pseudo
void demande_pseudo(char *username);
// Creation de l'entete du serveur
uint16_t create_header(uint8_t codereq_client)
// Remplissage entete buffer
void header_username_buffer(char *buf, uint16_t header_client, char *username);
//Connexion au serveur
void connexion_server(char *hostname, char *port, int sock) ;
//Requete d'inscription
void inscription_request(char *hostname, char *port);

#endif