CC = gcc 
CFLAGS = -g -Wall -O2

test: test.o libcosm.o
	$(CC) -o test libcosm.o test.o

libcosm.o: 
	$(CC) -c $(CFLAGS) ../libcosm.c

test.o: 
	$(CC) -c $(CFLAGS) test.c
