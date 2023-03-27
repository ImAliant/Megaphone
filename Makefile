CC = gcc
CFLAGS = -Wall -lpthread

CLIENT_SRC = src/client.c src/socket.c src/func/func_client.c
SERVEUR_SRC = src/serveur.c src/socket.c src/func/func_serveur.c

CLIENT_HEADERS = headers/socket.h headers/func/func_client.h
SERVEUR_HEADERS = headers/socket.h headers/func/func_serveur.h headers/users.h

OBJ = $(CLIENT_SRC:.c=.o)

all: client serveur

client: $(CLIENT_SRC) src/client.o $(CLIENT_HEADERS)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o client

serveur: $(SERVEUR_SRC) src/serveur.o $(SERVEUR_HEADERS)
	$(CC) $(CFLAGS) $(SERVEUR_SRC) -o serveur

clean:
	rm -f $(OBJ) src/serveur.o client serveur

.PHONY: clean