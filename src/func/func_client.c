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
#include "../../headers/message.h"

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
    recv_send_message(sock, buf, sizeof(header_client)+strlen(username), SEND);

    // RECEPTION ENTETE + ID
    recv_send_message(sock, buf, SIZE_MESS, RECV);

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

    lendata = strlen(data)-1;

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
    recv_send_message(sock, buf, size_buf, SEND);

    // RECEPTION DE LA REPONSE
    recv_send_message(sock, buf, SIZE_MESS*2, RECV);

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

int get_billets_request(char *hostname, char *port) {
    int sock;
    uint16_t header, id, numfil, nb;
    uint8_t codereq, lendata;

    size_t sizebuf = sizeof(uint16_t)+sizeof(uint16_t)+sizeof(uint16_t);
    char buf[sizebuf];
    memset(buf, 0, sizebuf);

    fflush(stdout);
    printf("IDENTIFIANT, NUMERO DU FIL ET NB DE BILLETS : ");
    int iduser, f, n;
    int r = scanf("%d %d %d", &iduser, &f, &n);
    if (r != 3) {
        perror("Erreur : Veuillez entrer 3 valeurs.\n");
        exit(1);
    }

    if (iduser > MAX_VALUE_11BITS_INTEGER) {
        perror("Erreur : Cette identifiant ne peux pas exister.\n");
        exit(1);
    }

    if (f < 0 || n < 0) {
        perror("Erreur : Les valeurs doivent être supérieures ou égales à zéro.\n");
        exit(1);
    }

    id = iduser;
    numfil = f;
    nb = n;

    // CONTRUCTION ENTETE
    codereq = REQ_GET_BILLET;
    header = htons((id << 5) | (codereq & 0x1F));
    numfil = htons(numfil);
    nb = htons(nb);

    // CONSTRUCTION DU BUFFER
    char *ptr = buf;
    memcpy(ptr, &header, sizeof(header));
    ptr += sizeof(header);
    memcpy(ptr, &numfil, sizeof(numfil));
    ptr += sizeof(numfil);
    memcpy(ptr, &nb, sizeof(nb));

    sock = connexion_server(hostname, port);

    // ENVOI DE LA REQUETE
    recv_send_message(sock, buf, sizebuf, SEND);

    // RECEPTION DE LA REPONSE
    // REPONSE PEUT ÊTRE UNE ERREUR
    recv_send_message(sock, buf, sizebuf, RECV);

    if (error_request(buf) == 1) {
        close(sock);
        exit(5);
    }

    // DECODAGE DE LA REPONSE
    ptr = buf;
    memcpy(&header, ptr, sizeof(header));
    ptr += sizeof(header);
    memcpy(&numfil, ptr, sizeof(numfil));
    ptr += sizeof(numfil);
    memcpy(&nb, ptr, sizeof(nb));

    codereq = ntohs(header) & 0x1F;
    id = ntohs(header) >> 5;
    numfil = ntohs(numfil);
    nb = ntohs(nb);

    // AFFICHAGE DE LA REPONSE
    printf("REPONSE : CODEREQ %hd, ID %hd, NUMFIL %hd, NB %hd\n", codereq, id, numfil, nb);

    // RECEPTION DES BILLETS
    size_t sizebillet = sizeof(uint16_t)+(MAX_USERNAME_LEN+1)*2+sizeof(uint8_t)+(SIZE_MESS+1);
    char billet[sizebillet];
    char data[SIZE_MESS+1];
    char pseudo_fil[MAX_USERNAME_LEN+1];
    char pseudo_billet[MAX_USERNAME_LEN+1];

    memset(billet, 0, sizebillet);
    
    int nb_billets = nb;
    for (int i = 0; i < nb_billets; i++) {
        memset(pseudo_fil, 0, MAX_USERNAME_LEN+1);
        memset(pseudo_billet, 0, MAX_USERNAME_LEN+1);
        memset(data, 0, SIZE_MESS+1);

        recv_send_message(sock, billet, sizebillet, RECV);

        // DECODAGE DU BILLET
        ptr = billet;
        memcpy(&numfil, ptr, sizeof(uint16_t));
        ptr += sizeof(uint16_t);
        memcpy(pseudo_fil, ptr, strlen(ptr) + 1);
        pseudo_fil[MAX_USERNAME_LEN] = '\0';
        ptr += strlen(ptr) + 1;
        memcpy(pseudo_billet, ptr, strlen(ptr) + 1);
        pseudo_billet[MAX_USERNAME_LEN] = '\0';
        ptr += strlen(ptr) + 1;
        memcpy(&lendata, ptr, sizeof(uint8_t));
        ptr += sizeof(uint8_t);
        memcpy(data, ptr, strlen(ptr) + 1);
        data[lendata] = '\0';

        numfil = ntohs(numfil);

        // AFFICHAGE DU BILLET
        printf("BILLET %d : NUMFIL %hd, ORIGINE %s, PSEUDO %s, DATA %s\n", i+1, numfil, pseudo_fil, pseudo_billet, data);
    }

    close(sock);

    return 0;
}

int subscribe_request(char *hostname, char *port){
     int sock;
    uint16_t header, id, numfil, nb,addr_Multicast;
    uint8_t codereq_client, lendata;
    char data[SIZE_MESS];
    

    fflush(stdout);
    printf("Entrez l'ID et le numero de fil : ");
    scanf("%hd %hd",&id,&numfil);

    if (id > MAX_VALUE_11BITS_INTEGER) {
        perror("Erreur : Cette identifiant ne peux pas exister.");
        exit(1);
    }

    if (numfil < 0) {
        perror("Erreur : Ce numéro de fil ne peux pas exister.");
        exit(1);
    }

    codereq_client = REQ_SUBSCRIBE;
    nb = 0;

    fflush(stdout);
    

    lendata = 0;

    char buf[SIZE_MESS*4];
    header = htons((id << 5) | (codereq_client & 0x1F));
    numfil = htons(numfil);
    nb = htons(nb);

    // CONSTRUCTION DE L'ENTETE
    memcpy(buf, &header, sizeof(header));
    memcpy(buf+sizeof(header), &numfil, sizeof(numfil));
    memcpy(buf+sizeof(header)+sizeof(numfil), &nb, sizeof(nb));
    memcpy(buf+sizeof(header)+sizeof(numfil)+sizeof(nb), &lendata, sizeof(lendata));
    memcpy(buf+sizeof(header)+sizeof(numfil)+sizeof(nb)+sizeof(lendata), data, lendata);
    size_t sizebuf = sizeof(header)+sizeof(numfil)+sizeof(nb)+sizeof(lendata)+lendata;
    // ENVOI DE LA REQUETE
    recv_send_message(sock, buf, sizebuf, SEND);

    // RECEPTION DE LA REPONSE
   
    recv_send_message(sock, buf, sizebuf, RECV);

    if (error_request(buf) == 1) {
        close(sock);
        exit(5);
    }

    memcpy(&header, buf, sizeof(header));
    memcpy(&numfil, buf+sizeof(header), sizeof(numfil));
    memcpy(&nb, buf+sizeof(header)+sizeof(numfil), sizeof(nb));
    id = ntohs(header) >> 5;
    codereq_client = ntohs(header) & 0x1F;
    numfil = ntohs(numfil);
    addr_Multicast = ntohs(addr_Multicast);
    // AFFICHAGE DE LA REPONSE
    printf("REPONSE : CODEREQ %hd, ID %hd, NUMFIL %hd,ADDRMLD %hd\n", codereq_client, id, numfil,addr_Multicast);

    char addr_Multicast_str[INET6_ADDRSTRLEN];
    snprintf(addr_Multicast_str,"%u", addr_Multicast);
    // abonnement au groupe multicast
    struct sockaddr_in6 grsock;
    memset(&grsock, 0, sizeof(grsock));
    grsock.sin6_family = AF_INET6;
    grsock.sin6_addr = in6addr_any;
    grsock.sin6_port = htons(atoi(port));

    int sock_udp = socket(AF_INET6, SOCK_DGRAM, 0);
    if(sock_udp < 0) 
    {
        perror("erreur socket");
        exit(1);
    }
    // pour l'envoi de messages(notifications) en multicast 
    if(bind(sock_udp, (struct sockaddr*)&grsock, sizeof(grsock))) 
    {
        perror("erreur bind");
        exit(1);
    } 
    struct ipv6_mreq mreq;
    mreq.ipv6mr_interface = 0;
    inet_pton(AF_INET6, addr_Multicast_str, &mreq.ipv6mr_multiaddr.s6_addr);
    if(setsockopt(sock_udp, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) < 0) 
    {
        perror("erreur abonnement groupe ");
    
    }

    close(sock);
    close(sock_udp);

    return 0;


}