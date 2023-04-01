CC = gcc
CFLAGS = -Wall -lpthread

CLIENT_SRC = src/client.c src/socket.c src/func/func_client.c
SERVEUR_SRC = src/serveur.c src/socket.c src/func/func_serveur.c

CLIENT_HEADERS = headers/socket.h headers/func/func_client.h headers/request_definition.h
SERVEUR_HEADERS = headers/socket.h headers/func/func_serveur.h headers/users.h headers/request_definition.h

OBJ = $(CLIENT_SRC:.c=.o)

all: client serveur

test: client_test serveur_test

client: $(CLIENT_SRC) src/client.o $(CLIENT_HEADERS)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o client

serveur: $(SERVEUR_SRC) src/serveur.o $(SERVEUR_HEADERS)
	$(CC) $(CFLAGS) $(SERVEUR_SRC) -o serveur

client_test: test/client_test.c headers/socket.h
	$(CC) $(CFLAGS) src/socket.c test/client_test.c -o client_test

serveur_test: test/serveur_test.c headers/socket.h headers/users.h
	$(CC) $(CFLAGS) src/socket.c test/serveur_test.c -o serveur_test

clean:
	rm -f $(OBJ) src/serveur.o client serveur
	rm -f client_test.o serveur_test.o client_test serveur_test

.PHONY: clean