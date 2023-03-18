CC = gcc
OBJ = src/client.o

client : $(OBJ)
	$(CC) -Wall -o $@ $^ $(CFLAGS)

clean:
	@echo "Nettoyage ..."
	@rm -rf $(OBJ) client
	@echo "Nettoyage terminé !"

.PHONY: clean