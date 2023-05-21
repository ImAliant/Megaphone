#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "message.h"

int recv_uint16(int sock, uint16_t *uint16) {
    uint16_t buf;

    int r = recv(sock, &buf, 2, 0);
    if (r < 0) return r;
    if (r < 2) return -2;

    *uint16 = ntohs(buf);

    return r;
}

int send_uint16(int sock, uint16_t uint16) {
    uint16_t buf = htons(uint16);
    int r = send(sock, &buf, 2, 0);
    if (r < 0) return r;
    if (r < 2) return -2;

    return r;
}

int recv_uint8(int sock, uint8_t *uint8) {
    int r = recv(sock, uint8, 1, 0);
    if (r < 0) return r;
    if (r < 1) return -2;

    return r;
}

int send_uint8(int sock, uint8_t uint8) {
    int r = send(sock, &uint8, 1, 0);
    if (r < 0) return r;
    if (r < 1) return -2;

    return r;
}

int recv_raw(int sock, void *buf, size_t size) {
    int r = recv(sock, buf, size, 0);

    if (r < 0) {
        perror("recv");
        return r;
    }

    if (r == 0) return r;
    if (r < (int)size) return -2;

    return r;
}

int send_raw(int sock, const void *buf, size_t size) {
    int r = send(sock, buf, size, 0);
    if (r < 0) return r;
    if (r < (int)size) return -2;

    return r;
}
