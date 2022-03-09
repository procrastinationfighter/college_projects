all: contamination

contamination: main.c contamination.o
		gcc -std=c99 -o contamination main.c contamination.o -lm

contamination.o:
		as -o contamination.o contamination.s

clean:
		rm contamination contamination.o