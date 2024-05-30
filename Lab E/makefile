all: exec

exec: elf
	gcc -g -m32 myELF.o -o myELF

elf: myELF.c
	gcc -g -m32 -c -o myELF.o myELF.c


.PHONY: clean
clean:
	rm -rf  myELF