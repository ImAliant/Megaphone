CC = gcc
HEADERS = headers/socket.h
OBJ_CLIENT = src/client.o
OBJ_SERVEUR = src/serveur.o
OBJ = src/client.o src/serveur.o
OBJ_DIR = obj

all: client serveur

client : $(OBJ_CLIENT) $(HEADERS)
	$(CC) -Wall -o $@ $^ -lpthread

serveur : $(OBJ_SERVEUR) $(HEADERS)
	$(CC) -Wall -o $@ $^ -lpthread

clean:
	@echo "Nettoyage ..."
	@rm -rf $(OBJ) client serveur
	@echo "Nettoyage terminé !"

.PHONY: clean