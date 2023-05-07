#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>

typedef enum {
    RECV,
    SEND,
} message_t;

int recv_send_message(int, char *, size_t, message_t);

#endif
