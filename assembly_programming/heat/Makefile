all: heat

heat: main.c heat.o
		gcc -o heat main.c heat.o

heat.o:
		nasm -f elf64 heat.asm

clean:
	rm heat heat.o