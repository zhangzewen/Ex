CFLAGS = -g -Wall -Werror -pthread -I ../../core/   \
					                         -I../../http/

Objs = ../../core/http_error.o \
			 ../../http/http_parse.o

main: main.o ${Objs}
	gcc ${CFLAGS} main.o ${Objs} -o main

main.o: main.c
	gcc -c ${CFLAGS} main.c -o main.o
clean:
	rm -fr *.o
	rm -fr main
	rm -fr core.*
