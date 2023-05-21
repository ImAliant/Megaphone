#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>

#define SEND 1
#define RECV 0

int recv_send_message(int, char*, size_t, int);
int recv_send_message_UDP(int,char *,size_t,struct sockaddr * ,socklen_t ,int);

#endif