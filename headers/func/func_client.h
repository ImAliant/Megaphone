#ifndef FUNC_CLIENT_H
#define FUNC_CLIENT_H

// Reponse a la demande d'inscription du serveur
int rep_demande_inscription(int);

// Reception d'une demande de la part du serveur
void recep_demande(int);

// Envoi du pseudo au serveur
void envoie_pseudo(int, char*);

// Envoi de l'id au serveur
void envoie_id(int, char*);

// Saisie du pseudo
void saisie_pseudo(char*);

// Saisie de l'id
void saisie_id(char*);

// Reception de l'id envoyé par le serveur
void recep_id(int, char*);

#endif