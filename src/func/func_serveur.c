#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "../../headers/socket.h"
#include "../../headers/users.h"
#include "../../headers/billet.h"
#include "../../headers/error.h"
#include "../../headers/func/func_serveur.h"
#include "../../headers/request.h"

#define SIZE_MESS 200
#define MAX_USERS 100
#define MAX_USERNAME_LEN 10
#define ID_BITS 11

int generate_user_id(char *username) {
    // HASH
    uint16_t id = 0;
    unsigned long hash = 5381;
    int c;
    while ((c = *username++)) {
        hash = ((hash << 5) + hash) + c;
    }

    for (int i = 0; i < ID_BITS; i++) {
        int bit = (hash >> i) & 1;
        id |= bit << i;
    }

    return id;
}

void create_new_user(char *username, int user_id, utilisateur liste[], int nb_utilisateurs) {
    utilisateur user;
    user.pseudo = malloc(sizeof(char*)*MAX_USERS);
    strcpy(user.pseudo, username);
    user.id = user_id;

    liste[nb_utilisateurs] = user;
    nb_utilisateurs++;
}

int recv_header_client(int sock_client, char *buf) {
    int r = recv(sock_client, buf, SIZE_MESS*2, 0);
    if (r < 0){
        perror("recv");
        close(sock_client);
        exit(1);
    }
    if(r == 0){
        fprintf(stderr, "send du client nul\n");
        close(sock_client);
        exit(1);
    }

    return 0;
}

int is_pseudo_used(char *username, utilisateur liste[], int nb_utilisateurs) {
    for(int i = 0; i < nb_utilisateurs; i++) {
        if(strcmp(liste[i].pseudo, username) == 0) {
            return 1;
        }
    }
    return 0;
}

int inscription_request(int sock_client, char *buf, utilisateur liste[], int nb_utilisateurs) {
    uint16_t header, id;
    uint8_t codereq;
    char username[MAX_USERNAME_LEN+1];

    memcpy(&header, buf, sizeof(uint16_t));
    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;
    memcpy(username, buf+sizeof(uint16_t), MAX_USERNAME_LEN);
    username[MAX_USERNAME_LEN] = '\0';

    // TEST SI LE NOMBRE MAX D'UTILISATEURS EST ATTEINT
    if (nb_utilisateurs >= MAX_USERS) {
        error_request(sock_client, codereq, id, ERR_MAX_USERS_REACHED);
        return 1;
    }

    // TEST SI LE PSEUDONYME EST DEJA UTILISE
    if(is_pseudo_used(username, liste, nb_utilisateurs)) {
        error_request(sock_client, codereq, id, ERR_PSEUDO_ALREADY_USED);
        return 1;
    }

    // AFFICHAGE DU MESSAGE DU CLIENT
    printf("CODEREQ %hd, ID %hd, PSEUDONYME %s\n", codereq, id, username);

    id = generate_user_id(username);

    create_new_user(username, id, liste, nb_utilisateurs);

    codereq = REQ_INSCRIPTION;

    header = htons((id << 5) | (codereq & 0x1F));
                
    memset(buf, 0, SIZE_MESS);
    memcpy(buf, &header, sizeof(header));

    int r = send(sock_client, buf, SIZE_MESS, 0);
    if (r < 0){
        perror("send");
        close(sock_client);
        exit(1);
    }

    return 0;
}
int get_nBillets_request(int sock_client, char *buf, struct fils *fils) {
   // TRADUCTION DU MESSAGE DU CLIENT
    uint16_t header, id, numfil, nb;
    uint8_t codereq, lendata;
    char data[SIZE_MESS+1];
    memset(data, 0, SIZE_MESS);

    // RECUPERATION DE L'ENTETE
    memcpy(&header, buf, sizeof(uint16_t));
    memcpy(&numfil, buf+sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&nb, buf+sizeof(uint16_t)*2, sizeof(uint16_t));
    memcpy(&lendata, buf+sizeof(uint16_t)*3, sizeof(uint8_t));
    memcpy(data, buf+sizeof(uint16_t)*3+sizeof(uint8_t), lendata);

    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;
    numfil = ntohs(numfil);
    nb = ntohs(nb);

    // AFFICHAGE DU MESSAGE DU CLIENT
    printf("CODEREQ %hd, ID %hd, NUMFIL %hd, NB %hd, LENDATA %hd, DATA %s", codereq, id, numfil, nb, lendata, data);

    //NUMERO DU FIL ET NOMBRE DE BILLETS SONT 0
    if(numfil == 0 && nb == 0){
        // On envoie le nombre de billets disponibles dans chaque fil
        //afficher le nombre totale des billets envoyé au client
        
      
        for(int i=0;i<fils->nb_fil;i++){
            
            
            codereq = REQ_GET_BILLET;
            header = htons((id << 5) | (codereq & 0x1F));
            numfil = htons(numfil);
            nb = htons(0);

            memset(buf, 0, SIZE_MESS*2);
            memcpy(buf, &header, sizeof(uint16_t));
            memcpy(buf+sizeof(uint16_t), &numfil, sizeof(uint16_t));
            memcpy(buf+sizeof(uint16_t)*2, &nb, sizeof(uint16_t));
            snprintf(buf,SIZE_MESS+sizeof(u_int16_t)*3 ,"le fil %d contient %d billets\n", i, fils->list_fil[i].nb_billet);
            if(send(sock_client, buf, SIZE_MESS+sizeof(uint16_t)*3, 0)==-1)
            {
                perror("send");
                close(sock_client);
                exit(1);
            }
            for(int j=0;j<fils->list_fil[i].nb_billet;j++){
               // envoyer tous les billets du fil i
                
               codereq = REQ_GET_BILLET;
                header = htons((id << 5) | (codereq & 0x1F));
                numfil = htons(numfil);
                nb = htons(0);

                memset(buf, 0, SIZE_MESS*2);
                memcpy(buf, &header, sizeof(uint16_t));
                memcpy(buf+sizeof(uint16_t), &numfil, sizeof(uint16_t));
                memcpy(buf+sizeof(uint16_t)*2, &nb, sizeof(uint16_t));
                snprintf(buf,SIZE_MESS+sizeof(uint16_t)*3,"le billet %d est %s \n", j, fils->list_fil[i].billets[j].contenu);
                if( send(sock_client, buf, SIZE_MESS+sizeof(uint16_t)*3, 0) == -1){
                perror("send");
                return 1;
            }
            }
        }
      }else if (numfil==0 && nb!=0){
        // on envoie les n dernier billets de chaque fil
        for(int i =0;i<fils ->nb_fil;i++){
            for(int j=nb;j>=fils->list_fil[i].nb_billet - nb;j--){
               
                codereq = REQ_GET_BILLET;
                header = htons((id << 5) | (codereq & 0x1F));
                numfil = htons(numfil);
                nb = htons(0);

                memset(buf, 0, SIZE_MESS*2);
                memcpy(buf, &header, sizeof(uint16_t));
                memcpy(buf+sizeof(uint16_t), &numfil, sizeof(uint16_t));
                memcpy(buf+sizeof(uint16_t)*2, &nb, sizeof(uint16_t));
                snprintf(buf,SIZE_MESS*2 ,"le billet %d est %s \n", j, fils->list_fil[i].billets[j].contenu);
                if( send(sock_client, buf, SIZE_MESS+sizeof(uint16_t)*3, 0) == -1){
                    perror("send");
                    return 1;
                }
            }
        }

      }else if (numfil!=0 && nb==0){
        // on envoie tous les billets du fil numfil
        for(int i=0;i<fils->list_fil[numfil].nb_billet;i++){
            
            codereq = REQ_GET_BILLET;
            header = htons((id << 5) | (codereq & 0x1F));
            numfil = htons(numfil);
            nb = htons(0);

            memset(buf, 0, SIZE_MESS*2);
            memcpy(buf, &header, sizeof(uint16_t));
            memcpy(buf+sizeof(uint16_t), &numfil, sizeof(uint16_t));
            memcpy(buf+sizeof(uint16_t)*2, &nb, sizeof(uint16_t));
            snprintf(buf,SIZE_MESS*2 ,"le billet %d est %s \n", i, fils->list_fil[numfil].billets[i].contenu);
           if( send(sock_client, buf, sizeof(uint16_t)*3, 0) == -1){
                perror("send");
                return 1;
            }
         
        }
    }
    else {
        // on envoie les n dernier billets du fil numfil
        for(int i=nb;i>=fils->list_fil[numfil].nb_billet - nb;i--){
            
            codereq = REQ_GET_BILLET;
            header = htons((id << 5) | (codereq & 0x1F));
            numfil = htons(numfil);
            nb = htons(0);

            memset(buf, 0, SIZE_MESS*2);
            memcpy(buf, &header, sizeof(uint16_t));
            memcpy(buf+sizeof(uint16_t), &numfil, sizeof(uint16_t));
            memcpy(buf+sizeof(uint16_t)*2, &nb, sizeof(uint16_t));
            snprintf(buf,SIZE_MESS*2 ,"le billet %d est %s \n", i, fils->list_fil[numfil].billets[i].contenu);

            if( send(sock_client, buf, SIZE_MESS+sizeof(uint16_t)*3, 0) == -1){
                perror("send");
                return 1;
            }
        }
    }
    return 0;
}
int post_billet_request(int sock_client, char *buf, struct fils *fils) {
    // TRADUCTION DU MESSAGE DU CLIENT
    uint16_t header, id, numfil, nb;
    uint8_t codereq, lendata;
    char data[SIZE_MESS+1];
    memset(data, 0, SIZE_MESS);

    // RECUPERATION DE L'ENTETE
    memcpy(&header, buf, sizeof(uint16_t));
    memcpy(&numfil, buf+sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&nb, buf+sizeof(uint16_t)*2, sizeof(uint16_t));
    memcpy(&lendata, buf+sizeof(uint16_t)*3, sizeof(uint8_t));
    memcpy(data, buf+sizeof(uint16_t)*3+sizeof(uint8_t), lendata);

    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;
    numfil = ntohs(numfil);
    nb = ntohs(nb);

    // AFFICHAGE DU MESSAGE DU CLIENT
    printf("CODEREQ %hd, ID %hd, NUMFIL %hd, NB %hd, LENDATA %hd, DATA %s", codereq, id, numfil, nb, lendata, data);

    // SI LE NUMERO DE FIL EST 0, CREER UN NOUVEAU FIL
    if (numfil == 0) {
        numfil = create_fil(fils, id, lendata, data);

        if (numfil == -1) {
            error_request(sock_client, codereq, id, ERR_MAX_FILS_REACHED);
            return 1;
        }
    }

    // SINON, AJOUTER LE BILLET AU FIL
    else {
        // TEST SI LE NUMERO DE FIL EST VALIDE
        if (numfil > fils->nb_fil) {
            error_request(sock_client, codereq, id, ERR_NUMFIL);
            return 1;
        }

        if (add_billet(fils, numfil, id, lendata, data) == -1) {
            error_request(sock_client, codereq, id, ERR_MAX_BILLETS_REACHED);
            return 1;
        }
    }

    // REPONSE AU CLIENT
    codereq = REQ_POST_BILLET;
    header = htons((id << 5) | (codereq & 0x1F));
    numfil = htons(numfil);
    nb = htons(0);

    memset(buf, 0, SIZE_MESS*2);
    memcpy(buf, &header, sizeof(uint16_t));
    memcpy(buf+sizeof(uint16_t), &numfil, sizeof(uint16_t));
    memcpy(buf+sizeof(uint16_t)*2, &nb, sizeof(uint16_t));

    if (send(sock_client, buf, sizeof(uint16_t)*3, 0) == -1) {
        perror("send");
        return 1;
    }

    return 0;
}

void error_request(int sock_client, uint8_t codereq_client, uint16_t id, int err) {
    uint16_t header_serv;
    char buf[SIZE_MESS];
    int r;

    switch (err)
    {
    case ERR_CODEREQ_UNKNOWN:
        printf("CODEREQ INCONNU : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_NON_ZERO_ID_WITH_CODE_REQ_ONE:
        printf("ID NON NUL AVEC CODEREQ 1, ID %d\n", id);
        break;
    case ERR_ID_DOES_NOT_EXIST:
        printf("ID INCONNU : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_PSEUDO_ALREADY_USED:
        printf("PSEUDONYME DEJA UTILISE : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_MAX_FILS_REACHED:
        printf("IMPOSSIBLE DE CREER UN NOUVEAU FIL : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_MAX_BILLETS_REACHED: 
        printf("IMPOSSIBLE DE CREER UN NOUVEAU BILLET : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_MAX_USERS_REACHED:
        printf("IMPOSSIBLE DE CREER UN NOUVEAU UTILISATEUR : <%d>, ID %d\n", codereq_client, id);
        break;
    case ERR_NUMFIL:
        printf("NUMERO DE FIL INCONNU : <%d>, ID %d\n", codereq_client, id);
        break;
    default:
        perror("code erreur inconnu");
        exit(EXIT_FAILURE);
    }
    
    codereq_client = 0;
    header_serv = htons((id << 5) | (codereq_client & 0x1F));
    
    memset(buf, 0, SIZE_MESS);
    memcpy(buf, &header_serv, sizeof(uint16_t));
    memcpy(buf+sizeof(uint16_t), &err, sizeof(int));
    
    r = send(sock_client, buf, SIZE_MESS, 0);
    if (r < 0){
        perror("send");
        close(sock_client);
        exit(1);
    }
}