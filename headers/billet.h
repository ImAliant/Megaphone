#ifndef BILLET_H
#define BILLET_H
#define MAX_LENGTH 450

struct billet {
    int idClient;
    int idThread;
    char contenu[MAX_LENGTH];
};

#endif 