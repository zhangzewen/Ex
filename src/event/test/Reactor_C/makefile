CFLAGS=-g3 -Wall -Werror
server: main.o event.o http_epoll.o
	gcc ${CFLAGS} main.o event.o http_epoll.o -g -o server
http_epoll.o: http_epoll.c http_epoll.h event_base.h 
	gcc -c ${CFLAGS} http_epoll.c -g -o http_epoll.o
event.o: http_epoll.h event.h event.c list.h event_base.h http_epoll.h
	gcc -c ${CFLAGS} event.c -g -o event.o
main.o: main.c event.h
	gcc -c ${CFLAGS} main.c -g -o main.o
clean:
	rm -fr *.o
	rm -fr main
	rm -fr core.*
