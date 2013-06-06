CFLAGS = -g -Wall -Werror -pthread -I ../../core/   \
					                         -I../../event/   \
																	 -I../../http/

Objs = ../../core/http_error.o \
			 ../../http/http_parse.o

server: server.o ${Objs}
	gcc ${CFLAGS} server.o ${Objs} -o server

server.o: server.c
	gcc -c ${CFLAGS} server.c -o server.o
clean:
	rm -fr *.o
	rm -fr response
	rm -fr core.*
	rm -fr client
	rm -fr server
	rm -fr epoll_ex


