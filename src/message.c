#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "message.h"

static void print_bin8(const char *msg, uint8_t uint8) {
    if (msg) {
        printf("%-10s: ", msg);
    }

    for (size_t i = 0; i < 8; i++) {
        if (i % 8 == 0) printf(" ");
        printf("%c", (uint8 & (1 << (7 - i))) ? '1' : '0');
    }

    if (msg) {
        printf("\n");
    }
}

static void print_bin16(const char *msg, uint16_t uint16) {
    if (msg) {
        printf("%-10s: ", msg);
    }

    print_bin8(NULL, uint16 >> 8);
    printf(" ");
    print_bin8(NULL, uint16 % (1 << 8));

    if (msg) {
        printf("\n");
    }
}

int recv_uint16(int sock, uint16_t *uint16) {
    uint16_t buf;

    int r = recv(sock, &buf, 2, 0);
    if (r < 0) return r;
    if (r < 2) return -2;

    *uint16 = ntohs(buf);

    print_bin16("RECEIVED", *uint16);

    return r;
}

int send_uint16(int sock, uint16_t uint16) {
    print_bin16("SENDING", uint16);

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

    print_bin8("RECEIVED", *uint8);

    return r;
}

int send_uint8(int sock, uint8_t uint8) {
    print_bin8("SENDING", uint8);

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
