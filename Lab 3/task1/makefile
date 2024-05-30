all : util.o start.o
	gcc -g -Wall -m32 -o task1 start.o util.o
	
util.o : util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start.o : start.s
	nasm -f elf32 start.s -o start.o

.PHONY : clean

clean:
	rm -f *.o task1
