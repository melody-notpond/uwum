CC=gcc
CLIBS=-lxcb
CFLAGS=-Wall -Wextra -g
CODE=src/

all: $(CODE)*.c
	$(CC) $(CLIBS) $(CFLAGS) $? -o uwum

run:
	./run.sh

tty:
	./run-tty.sh
