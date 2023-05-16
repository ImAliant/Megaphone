#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "message.h"

int recv_send_message(int sock, char *buf, size_t size, message_t type) {
    if (type == RECV) {
        int r = recv(sock, buf, size, 0);
        if (r < 0) {
            perror("recv");
            return -1;
        }
        if (r == 0) {
            fprintf(stderr, "recv nul\n");
            return -1;
        }
    } else if (type == SEND) {
        int s = send(sock, buf, size, 0);
        if (s < 0) {
            perror("send");
            return -1;
        }
    }

    return 0;
}
