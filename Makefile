CC=gcc -std=c99 -Wall -pedantic
CFLAGS=-pipe -O2 -D_GNU_SOURCE
LDFLAGS=-lm

all: posty

posty: posty.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	@rm posty

.PHONY: all posty clean
