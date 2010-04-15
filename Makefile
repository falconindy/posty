CC=gcc -std=c99 -Wall -pedantic
CFLAGS=-pipe -O2 -D_GNU_SOURCE
LDFLAGS=-lm
#OBJ=stack.o

all: posty

posty: posty.c $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(OBJ) -o $@

#%.o: %.c %.h
#	$(CC) $(CFLAGS) $< -c

clean:
	@rm posty

.PHONY: all posty clean
