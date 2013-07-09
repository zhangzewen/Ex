CFLAGS = -g -Wall  -pthread -I ../../event/	\
				-I ../../core/ \
	

Objs =../../event/http_epoll.o ../../core/file.o ../../core/io.o


server: server.o ${Objs}
	gcc ${CFLAGS} server.o ${Objs} -o server

server.o: server.c
	gcc -c ${CFLAGS} server.c -o server.o
clean:
	rm -fr *.o
	rm -fr server
	rm -fr core.*
