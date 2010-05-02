CC=gcc -std=c99 -Wall -pedantic -g
CFLAGS=-pipe -D_GNU_SOURCE
LDFLAGS=-lm -lreadline

all: posty

posty: posty.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	@rm posty

.PHONY: all posty clean
