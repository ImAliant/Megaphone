#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../headers/message.h"

// sert a appeler send()
#define SEND 1
// sert a appeler recv()
#define RECV 0

int recv_send_message(int sock, char *buf, size_t size, int type) {
    if (type == RECV) {
        int r = recv(sock, buf, size, 0);
        if (r < 0){
            perror("recv");
            close(sock);
            exit(1);
        }
        if(r == 0){
            fprintf(stderr, "recv nul\n");
            close(sock);
            exit(1);
        }
    } else if (type == SEND) {
        int s = send(sock, buf, size, 0);
        if (s < 0){
            perror("send");
            close(sock);
            exit(1);
        }
    }

    return 0;
}

int recv_send_message_UDP(int sock, char *buf, size_t size,struct sockaddr * adr,
                                                            socklen_t addrlen ,int type) {
    if (type == RECV) {
        int r = recvfrom(sock, buf, size, 0,NULL,NULL);
        if (r < 0){
            perror("recv");
            close(sock);
            exit(1);
        }
        if(r == 0){
            fprintf(stderr, "recv nul\n");
            close(sock);
            exit(1);
        }
    } else if (type == SEND) {
        int s = sendto(sock, buf, size, 0,(struct sockaddr *)&adr,addrlen);
        if (s < 0){
            perror("send");
            close(sock);
            exit(1);
        }
    }

    return 0;
}