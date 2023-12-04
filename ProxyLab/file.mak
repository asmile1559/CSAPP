CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: test 

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

test.o: test.c csapp.h
	$(CC) $(CFLAGS) -c test.c

test: test.o csapp.o
	$(CC) $(CFLAGS) test.o csapp.o -o test $(LDFLAGS)

