server: main.o event.o http_epoll.o evbuf.o
	gcc -g3 main.o event.o http_epoll.o evbuf.o -g -o server
http_epoll.o: http_epoll.c http_epoll.h event_base.h 
	gcc -c -g3 http_epoll.c -g -o http_epoll.o
event.o: http_epoll.h event.h event.c list.h event_base.h http_epoll.h
	gcc -c -g3 event.c -g -o event.o
evbuf.o: evbuf.c evbuf.h
	gcc -c -g3 evbuf.c -o evbuf.o
http_request.o: http_request.c http_request.h http_buffer.h
	gcc -c -g3 http_request.c -o http_request.o
http_parse.o: http_parse.h http_request.h http_buffer.h
	gcc -c -g3 http_parse.h -o http_parse.o
main.o: main.c event.h evbuf.h
	gcc -c -g3 main.c -g -o main.o
clean:
	rm -fr *.o
	rm -fr server
	rm -fr core.*
