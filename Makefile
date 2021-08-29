bmpraw: endian.o
	gcc -o bmpraw bmpraw.c endian.o -largp

endian.o: endian.c endian.h