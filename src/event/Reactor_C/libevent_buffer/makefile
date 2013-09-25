main: main.o http_buffer.o
	gcc -g3 main.o http_buffer.o -o main
http_buffer.o: http_buffer.c http_buffer.h
	gcc -g3 -c http_buffer.c -o http_buffer.o
main.o: main.c http_buffer.h
	gcc -g3 -c main.c -o main.o
clean:
	rm -fr *.o
	rm -fr main
	rm -fr core.*
