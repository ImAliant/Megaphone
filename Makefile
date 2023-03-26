CC = gcc
CFLAGS = -Wall -lpthread

CLIENT_SRC = src/client.c src/socket.c
OBJ = $(CLIENT_SRC:.c=.o)

CLIENT_EXE = client
SERVEUR_SRC = serveur

all: client serveur

client: $(CLIENT_SRC) src/client.o headers/socket.h
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o client

serveur: $(SERVEUR_SRC) src/serveur.o headers/socket.h
	$(CC) $(CFLAGS) src/serveur.c src/socket.c -o serveur

clean:
	rm -f $(OBJ) src/serveur.o client serveur

.PHONY: clean