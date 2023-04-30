CC = gcc
CFLAGS = -Wall -pthread

SRC = src/socket.c src/message.c
CLIENT_SRC = src/client.c src/func/func_client.c $(SRC)
SERVEUR_SRC = src/serveur.c src/billet.c src/func/func_serveur.c $(SRC)

HEADERS = headers/socket.h headers/message.h headers/request.h 
CLIENT_HEADERS = $(HEADERS) headers/func/func_client.h
SERVEUR_HEADERS = $(HEADERS) headers/billet.h headers/func/func_serveur.h headers/users.h

all: client serveur

client: $(CLIENT_SRC) $(CLIENT_HEADERS)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o client

serveur: $(SERVEUR_SRC) $(SERVEUR_HEADERS)
	$(CC) $(CFLAGS) $(SERVEUR_SRC) -o serveur

clean:
	rm -f client serveur

.PHONY: clean