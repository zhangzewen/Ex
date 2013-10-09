server: main.o event.o http_epoll.o evbuf.o evutil.o
	gcc -g3 main.o event.o http_epoll.o evbuf.o evutil.o -g3 -o server
http_epoll.o: http_epoll.c http_epoll.h event_base.h  evutil.h
	gcc -c -g3 http_epoll.c -g -o http_epoll.o
event.o: http_epoll.h event.h event.c list.h event_base.h http_epoll.h evutil.h min_heap.h
	gcc -c -g3 event.c -g -o event.o
evbuf.o: evbuf.c evbuf.h
	gcc -c -g3 evbuf.c -o evbuf.o
evutil.o: evutil.h evutil.c 
	gcc -c -g3 evutil.c -o evutil.o
main.o: main.c event.h evbuf.h  
	gcc -c -g3 main.c -g -o main.o
clean:
	rm -fr *.o
	rm -fr server
	rm -fr core.*
