all: gray

gray: main.c gray.o
		gcc -std=c99 -o gray main.c gray.o

gray.o:
		as -o gray.o gray.s

clean:
	rm gray gray.o