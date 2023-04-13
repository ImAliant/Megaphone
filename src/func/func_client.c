#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#include "../../headers/socket.h"
#include "../../headers/error.h"
#include "../../headers/func/func_client.h"
#include "../../headers/request.h"
#include "../../headers/billet.h"

#define SIZE_MESS 100
#define MAX_USERNAME_LEN 10
#define MAX_VALUE_11BITS_INTEGER 2047

int error_request(char *buf) {
    uint16_t header;
    uint8_t codereq;
    int type;

    memcpy(&header, buf, sizeof(header));
    codereq = ntohs(header) & 0x1f;
    if (codereq > 0) {
        return 0;
    }

    memcpy(&type, buf+sizeof(uint16_t), sizeof(type));

    switch(type) {
        case ERR_CODEREQ_UNKNOWN:
            printf("ERREUR : REQUETE INCONNUE \n");
            break;
        case ERR_NON_ZERO_ID_WITH_CODE_REQ_ONE:
            printf("ERREUR : ID NON NUL AVEC CODEREQ=1 \n");
            break;
        case ERR_ID_DOES_NOT_EXIST:
            printf("ERREUR : ID INEXISTANT DANS LA TABLE \n");
            break;
        case ERR_PSEUDO_ALREADY_USED:
            printf("ERREUR : PSEUDO DEJA UTILISE \n");
            break;
        case ERR_MAX_FILS_REACHED:
            printf("ERREUR : IMPOSSIBLE DE CREER UN NOUVEAU FIL \n");
            break;
        case ERR_MAX_USERS_REACHED:
            printf("ERREUR : IMPOSSIBLE DE CREER UN NOUVEAU UTILISATEUR \n");
            break;
        case ERR_MAX_BILLETS_REACHED:
            printf("ERREUR : IMPOSSIBLE DE CREER UN NOUVEAU BILLET \n");
            break;
        case ERR_NUMFIL:
            printf("ERREUR : NUMERO DE FIL INEXISTANT \n");
            break;
        default:
            perror("ERREUR : ERREUR INCONNUE \n");
            exit(EXIT_FAILURE);
    }

    return 1;
}

void remove_special_chars(char *str) {
    int i, j;
    for (i = 0, j = 0; str[i]; i++) {
        if (!isspace(str[i]) && isprint(str[i])) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

void completion_pseudo(char *username) {
    int len = strlen(username);
    if (len < MAX_USERNAME_LEN) {
        memset(username+len, '#', MAX_USERNAME_LEN-len);
        username[MAX_USERNAME_LEN] = '\0';
        remove_special_chars(username);
    }
}

void demande_pseudo(char *username) {
    printf("Saisir votre pseudo : ");
    memset(username, 0, MAX_USERNAME_LEN+1);
    fgets(username, MAX_USERNAME_LEN, stdin);
    completion_pseudo(username);
    printf("PSEUDO : %s \n", username);
}

uint16_t create_header(uint8_t codereq_client) {
    uint16_t id = 0;
    uint16_t header_client = htons((id << 5) | (codereq_client & 0x1F));

    return header_client;
}

void header_username_buffer(char *buf, uint16_t header_client, char *username) {
    memcpy(buf, &header_client, sizeof(header_client));
    memcpy(buf+sizeof(header_client), username, strlen(username));
    buf[sizeof(header_client)+strlen(username)] = '\0';
}

int connexion_server(char *hostname, char *port) {
    int sock;
    struct sockaddr_in6* server_addr;
    int adrlen;

    switch (get_server_addr(hostname, port, &sock, &server_addr, &adrlen)) 
    {
    case 0: break;
    case -1:
        fprintf(stderr, "Erreur: hote non trouve.\n"); 
    case -2:
        fprintf(stderr, "Erreur: echec de creation de la socket.\n");
    exit(1);
    }

    return sock;
}

int inscription_request(char *hostname, char *port) {
    int sock;
    uint16_t header_client, header_serv, id_serv;
    uint8_t codereq_serv;
    char username[MAX_USERNAME_LEN+1];
    char buf[SIZE_MESS];
    
    codereq_serv = REQ_INSCRIPTION;
    
    header_client = create_header(codereq_serv);

    demande_pseudo(username);

    header_username_buffer(buf, header_client, username);

    sock = connexion_server(hostname, port);

    // ENVOI ENTETE + PSEUDO
    int ecrit = send(sock, buf, sizeof(header_client)+strlen(username), 0);
    if(ecrit <= 0){
        perror("erreur ecriture");
        close(sock);
        exit(3);
    }

    // RECEPTION ENTETE + ID
    memset(buf, 0, SIZE_MESS);
    int r = recv(sock, buf, SIZE_MESS, 0);
    if(r <= 0){
        perror("erreur lecture");
        close(sock);
        exit(4);
    }

    if (error_request(buf) == 1) {
        close(sock);
        exit(5);
    }

    // DECODAGE ENTETE + ID
    memcpy(&header_serv, buf, sizeof(header_serv));
    id_serv = ntohs(header_serv) >> 5;
    codereq_serv = ntohs(header_serv) & 0x1F;

    printf("VOICI VOTRE ID : %d \n", id_serv);

    close(sock);

    return 0;
}






//Demande de n billets pour un fil donné
int get_nBillets_request(char * hostname,char* port,struct fils* f){
    int sock;
   
    f=malloc(sizeof(struct fils));
    if(f==NULL){
        perror("Erreur allocation mémoire");
        exit(1);
    }
    uint16_t header,id, numfil, nb;
    uint8_t codereq_client ;
    uint8_t lendata = 0;
    char data[SIZE_MESS]="";

    fflush(stdout);
    printf("IDENTIFIANT , NUM FIL ET NB DE BILLETS : ");
    scanf("%hd %hd %hd", &id, &numfil, &nb);
    if (id > MAX_VALUE_11BITS_INTEGER) 
    {
        perror("Erreur : Cet identifiant ne peux pas exister.");
        exit(1);
    }
    if (numfil < 0) 
    {
        perror("Erreur : Ce numéro de fil ne peux pas exister.");
        exit(1);
    }
    if (nb < 0) 
    {
        perror("Erreur : Ce nombre de billets ne peux pas exister.");
        exit(1);
    }
    codereq_client = REQ_GET_BILLET;
    fflush(stdout);
    
char buf[SIZE_MESS*2];
    header = htons((id << 5) | (codereq_client & 0x1F));
    numfil = htons(numfil);
    nb = htons(nb);

    // CONSTRUCTION DE L'ENTETE
    memcpy(buf, &header, sizeof(header));
    memcpy(buf+sizeof(header), &numfil, sizeof(numfil));
    memcpy(buf+sizeof(header)+sizeof(numfil), &nb, sizeof(nb));
    memcpy(buf+sizeof(header)+sizeof(numfil)+sizeof(nb), &lendata, sizeof(lendata));
    memcpy(buf+sizeof(header)+sizeof(numfil)+sizeof(nb)+sizeof(lendata), data, lendata);

    size_t size_buf = sizeof(header)+sizeof(numfil)+sizeof(nb)+sizeof(lendata)+lendata;

    sock = connexion_server(hostname, port);

    // ENVOI DE LA REQUETE
    int ecrit = send(sock, buf,size_buf, 0);
    if (ecrit < 0){
        perror("send");
        close(sock);
        exit(1);
    }
    //reception de la reponse du serveur
   
        for(int i=0;i<f->nb_fil;i++){
            for(int j =0;j< f->list_fil[i].nb_billet;j++){
                memset(buf, 0, size_buf);
                int read = recv(sock, buf, size_buf, 0);
                
                if (read < 0)
                {
                    perror("recv");
                    close(sock);
                    exit(1);
                }    
                
                //afichage des billets
                printf("Billet %d du fil %d : %s\n",j+1,i+1,buf);
            
            }
       }
          
     close(sock);

    return 0;
        
    

    }
    




int post_billet_request(char *hostname, char *port) {
    int sock;
    uint16_t header, id, numfil, nb;
    uint8_t codereq_client, lendata;
    char data[SIZE_MESS];
    memset(data, 0, SIZE_MESS);

    fflush(stdout);
    printf("IDENTIFIANT ET NUM FIL (0 pour en créer un nouveau) : ");
    scanf("%hd %hd", &id, &numfil);

    if (id > MAX_VALUE_11BITS_INTEGER) {
        perror("Erreur : Cette identifiant ne peux pas exister.");
        exit(1);
    }

    if (numfil < 0) {
        perror("Erreur : Ce numéro de fil ne peux pas exister.");
        exit(1);
    }

    codereq_client = REQ_POST_BILLET;
    nb = 0;

    fflush(stdout);
    printf("ENTREZ VOTRE MESSAGE :\n");
    getchar();
    fgets(data, SIZE_MESS, stdin);

    lendata = strlen(data);

    char buf[SIZE_MESS*2];
    header = htons((id << 5) | (codereq_client & 0x1F));
    numfil = htons(numfil);
    nb = htons(nb);

    // CONSTRUCTION DE L'ENTETE
    memcpy(buf, &header, sizeof(header));
    memcpy(buf+sizeof(header), &numfil, sizeof(numfil));
    memcpy(buf+sizeof(header)+sizeof(numfil), &nb, sizeof(nb));
    memcpy(buf+sizeof(header)+sizeof(numfil)+sizeof(nb), &lendata, sizeof(lendata));
    memcpy(buf+sizeof(header)+sizeof(numfil)+sizeof(nb)+sizeof(lendata), data, lendata);

    size_t size_buf = sizeof(header)+sizeof(numfil)+sizeof(nb)+sizeof(lendata)+lendata;

    sock = connexion_server(hostname, port);

    // ENVOI DE LA REQUETE
    int ecrit = send(sock, buf, size_buf, 0);
    if (ecrit < 0){
        perror("send");
        close(sock);
        exit(1);
    }

    // RECEPTION DE LA REPONSE
    memset(buf, 0, SIZE_MESS*2);
    int read = recv(sock, buf, SIZE_MESS*2, 0);
    if (read < 0){
        perror("recv");
        close(sock);
        exit(1);
    }

    if (error_request(buf) == 1) {
        close(sock);
        exit(5);
    }

    // DECODAGE DE LA REPONSE
    memcpy(&header, buf, sizeof(header));
    memcpy(&numfil, buf+sizeof(header), sizeof(numfil));
    memcpy(&nb, buf+sizeof(header)+sizeof(numfil), sizeof(nb));
    id = ntohs(header) >> 5;
    codereq_client = ntohs(header) & 0x1F;
    numfil = ntohs(numfil);

    // AFFICHAGE DE LA REPONSE
    printf("REPONSE : CODEREQ %hd, ID %hd, NUMFIL %hd\n", codereq_client, id, numfil);

    close(sock);

    return 0;
}
