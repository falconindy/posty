CC      = gcc -std=c99 -Wall -pedantic -g
CFLAGS  = -pipe -O2 -D_GNU_SOURCE
LDFLAGS = -lm -lreadline

all: posty

posty: posty.c posty.h
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	@rm -f posty

.PHONY: all clean
