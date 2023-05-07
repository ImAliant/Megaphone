BUILD ?= build
SRC = src
HEADERS = headers

CC ?= gcc
CFLAGS = -Wall -Wextra -Wpedantic -I$(HEADERS)

ALL = client serveur

.PHONY: all
all: $(ALL)

client: $(BUILD)/socket.o $(BUILD)/func/func_client.o $(SRC)/client.c
	$(CC) $(CFLAGS) $^ -o $@

serveur: $(BUILD)/socket.o $(BUILD)/func/func_serveur.o $(SRC)/serveur.c
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/%.o: $(SRC)/%.c $(HEADERS)/%.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(ALL) $(BUILD)

