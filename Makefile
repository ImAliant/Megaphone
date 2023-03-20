CC = gcc
OBJ_CLIENT = src/client.o
OBJ_SERVEUR = src/serveur.o
OBJ = src/client.o src/serveur.o
OBJ_DIR = obj

all: client serveur

client : $(OBJ_CLIENT)
	$(CC) -Wall -o $@ $^ $(CFLAGS)

serveur : $(OBJ_SERVEUR)
	$(CC) -Wall -o $@ $^ $(CFLAGS)

clean:
	@echo "Nettoyage ..."
	@rm -rf $(OBJ) client serveur
	@echo "Nettoyage terminé !"

.PHONY: clean