#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t codereq; // limité à 5 bits
    uint16_t id;     // limité à 11 bits
} client_header_t;

typedef struct {
    uint8_t codereq; // limité à 5 bits
    uint16_t id;     // limité à 11 bits
    uint16_t numfil;
    uint16_t nb;
} server_header_t;

int recv_uint16(int, uint16_t *);
int send_uint16(int, uint16_t);

int recv_uint8(int, uint8_t *);
int send_uint8(int, uint8_t);

int recv_bytes(int, size_t, void *);
int send_bytes(int, size_t, void *);

int recv_raw(int, void *, size_t);
int send_raw(int, const void *, size_t);

#endif
