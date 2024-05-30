all: my_loader

my_loader: task2.o start.o
	gcc -m32 -c task2.c -o task2.o
	ld -o my_loader task2.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

start.o: start.s
	nasm -g -f elf32 -o start.o start.s

.PHONY: clean

clean:
	rm -f task2.o start.o my_loader