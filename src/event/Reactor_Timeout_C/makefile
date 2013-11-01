server: main.o event.o http_epoll.o evbuf.o 
	gcc -g3 main.o event.o http_epoll.o evbuf.o RBTree.o timer.o -g3 -o server
http_epoll.o: http_epoll.c http_epoll.h event_base.h 
	gcc -c -g3 http_epoll.c -g -o http_epoll.o
event.o: http_epoll.h event.h event.c list.h event_base.h http_epoll.h RBTree.h timer.h
	gcc -c -g3 event.c -g -o event.o
evbuf.o: evbuf.c evbuf.h
	gcc -c -g3 evbuf.c -o evbuf.o
main.o: main.c event.h evbuf.h  
	gcc -c -g3 main.c -g -o main.o
timer.o: timer.c timer.h
	gcc -c -g3 timer.c -g -o timer.o
RBTree.o: RBTree.c RBTree.h
	gcc -c -g3 RBTree.c -g -o RBTree.o
clean:
	rm -fr *.o
	rm -fr server
	rm -fr core.*
