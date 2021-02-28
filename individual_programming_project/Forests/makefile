CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

.PHONY: all clean

all: forests

forests: main.o userInterface.o World.o BeingTree.o String.o
	$(CC) $(CFLAGS) -o $@ $^

World.o: World.c World.h String.h BeingTree.h
	$(CC) $(CFLAGS) -c $<

String.o: String.c String.h
	$(CC) $(CFLAGS) -c $<

BeingTree.o: BeingTree.c BeingTree.h String.h
	$(CC) $(CFLAGS) -c $<

userInterface.o: userInterface.c userInterface.h String.h World.h
	$(CC) $(CFLAGS) -c $<

main.o: main.c userInterface.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main

