menu_map : menu_map.o
	gcc -m32 -Wall -g menu_map.o -o menu_map

menu_map.o : menu_map.c
	gcc -g -Wall -m32 -c -o menu_map.o menu_map.c

.PHONY : clean

clean :
	rm -f *.o menu_map