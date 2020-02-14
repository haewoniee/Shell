CC = gcc
CFLAGS = -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -fstack-protector-all

PROGS = d8sh

.PHONY: all clean

all: $(PROGS)

clean:
	@rm -f *.o $(PROGS)

d8sh: d8sh.o executor.o lexer.o parser.tab.o
	$(CC) $(CFLAGS) -o d8sh d8sh.o executor.o lexer.o parser.tab.o

d8sh.o: executor.h lexer.h d8sh.c
	$(CC) $(CFLAGS) -c d8sh.c

executor.o: executor.h command.h executor.c
	$(CC) $(CFLAGS) -c executor.c

lexer.o: parser.tab.h lexer.c
	$(CC) $(CFLAGS) -c lexer.c

parser.tab.o: command.h parser.tab.c
	$(CC) $(CFLAGS) -c parser.tab.c
