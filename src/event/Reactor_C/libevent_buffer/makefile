main: main.o evbuf.o
	gcc -g3 main.o evbuf.o -o main
evbuf.o: evbuf.c evbuf.h
	gcc -g3 -c evbuf.c -o evbuf.o
main.o: main.c evbuf.h
	gcc -g3 -c main.c -o main.o
clean:
	rm -fr *.o
	rm -fr main
	rm -fr core.*
