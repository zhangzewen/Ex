CFLAGS = -g3 -Werror -Wall
LIB = main.o event.o http_epoll.o evbuf.o RBTree.o timer.o dns_util.o http_resolver.o hash.o vector.o
server: $(LIB)
	gcc $(CFLAGS) $(LIB) $(CFLAGS) -o server
http_epoll.o: http_epoll.c http_epoll.h event_base.h 
	gcc -c $(CFLAGS) http_epoll.c -g -o http_epoll.o
timer.o: timer.c timer.h
	gcc -c $(CFLAGS) timer.c -g -o timer.o
RBTree.o: RBTree.c RBTree.h
	gcc -c $(CFLAGS) RBTree.c -g -o RBTree.o
event.o: http_epoll.h event.h event.c list.h event_base.h http_epoll.h RBTree.h timer.h
	gcc -c $(CFLAGS) event.c -g -o event.o
evbuf.o: evbuf.c evbuf.h
	gcc -c $(CFLAGS) evbuf.c -o evbuf.o
http_resolver.o: http_resolver.h dns_util.h RBTree.h
	gcc -c $(CFLAGS) http_resolver.c -o http_resolver.o
dns_util.o: dns_util.h http_resolver.h
	gcc -c $(CFLAGS) dns_util.c  -o dns_util.o
hash.o: hash.h
	gcc -c $(CFLAGS) hash.c -o hash.o
vector.o: vector.h
	gcc -c ${CFLAGS} vector.c -o vector.o
main.o: main.c event.h evbuf.h http_resolver.h
	gcc -c $(CFLAGS) main.c -g -o main.o
clean:
	rm -fr *.o
	rm -fr server
	rm -fr core.*
