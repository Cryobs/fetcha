# Makefile for fetcha

CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC = src/main.c
BIN = bin/main

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(BIN)

