#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>

/* [SERVEUR] */

// Creation de la socket
int creation_socket();

// Creation de l'adresse du destinataire (serveur)
void addresse_destinataire(int, struct sockaddr_in6*);

// Desactiver l'option n'accepter que de l'IPv6
void desac_option_only_ipv6(int);

// Le numero de port peut etre utilise en parallele
void parallel_use_port(int);

// On associe la socket au port
void bind_port(int, struct sockaddr_in6*);

// On ecoute sur le port
void listen_port(int sock);

/* [CLIENT] */

// On affiche l'adresse du destinataire
//void affiche_adresse(struct sockaddr_in6*);

// On recupere l'adresse du serveur
//int get_server_addr(char*, char*, int*, struct sockaddr_in6**, int*);

#endif