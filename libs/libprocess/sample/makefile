CC = gcc 
CFLAGS = -g -Wall -O2

testproc: sample.o process.o
	$(CC) -o testproc sample.o process.o

process.o: process.c process.h
	$(CC) -c $(CFLAGS) process.c -o process.o

sample.o: sample.c process.h 
	$(CC) -c $(CFLAGS) sample.c -o sample.o
