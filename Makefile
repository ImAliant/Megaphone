CC = gcc
CFLAGS = -Wall -lpthread

CLIENT_SRC = src/client.c src/socket.c src/func/func_client.c
SERVEUR_SRC = src/serveur.c src/socket.c src/billet.c src/func/func_serveur.c

CLIENT_HEADERS = headers/socket.h headers/func/func_client.h headers/request.h
SERVEUR_HEADERS = headers/socket.h headers/billet.h headers/func/func_serveur.h headers/users.h headers/request.h

all: client serveur

client: $(CLIENT_SRC) $(CLIENT_HEADERS)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o client

serveur: $(SERVEUR_SRC) $(SERVEUR_HEADERS)
	$(CC) $(CFLAGS) $(SERVEUR_SRC) -o serveur

clean:
	rm -f client serveur

.PHONY: clean