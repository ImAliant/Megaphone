#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>

#define SIZE_MESS 100
#define NUM_THREADS 100

struct thread_data {
    int sockfd;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
};

void *serve(void *arg) {
    struct thread_data *data = (struct thread_data *) arg;
    int sockfd = data->sockfd;
    struct sockaddr_storage their_addr = data->their_addr;
    socklen_t addr_len = data->addr_len;
    char ipstr[INET6_ADDRSTRLEN];
    int numbytes;
    char buf[SIZE_MESS+1];

    printf("Thread started\n");
    
    while (1) {
        if ((numbytes = recvfrom(sockfd, buf, SIZE_MESS+1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        printf("got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), ipstr, sizeof ipstr));
        printf("packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        printf("packet contains \"%s\"\n", buf);
    }

    printf("Thread finished\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    /*int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in6 servadr;
    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    servadr.sin6_addr = in6addr_any;
    servadr.sin6_port = htons(atoi(argv[1]));

    int r = bind(sock, (struct sockaddr *) &servadr, sizeof(servadr));
    if (r < 0) return -1;

    char buffer[SIZE_MESS+1];
    struct sockaddr_in6 cliadr;
    socklen_t len = sizeof(cliadr);

    memset(buffer, 0, SIZE_MESS+1);
    r = recvfrom(sock, buffer, SIZE_MESS, 0, (struct sockaddr *) &cliadr, &len);
    if (r < 0) return -1;
    printf("message recu - %d octets : %s\n", r, buffer);

    memset(buffer, 0, SIZE_MESS+1);
    sprintf(buffer, "RECU");
    r = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &cliadr, len);
    if (r < 0) return -1;

    close(sock);

    return 0;*/
    struct addrinfo hints, *res, *p;
    int sockfd;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char ipstr[INET6_ADDRSTRLEN];
    pthread_t threads[NUM_THREADS];
    struct thread_data td[NUM_THREADS];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    int rv;
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(res);

    printf("listening on port %s\n", argv[1]);

    for (int i = 0; i < NUM_THREADS; )
}