CFLAGS = -g3 
server: main.o event.o http_epoll.o http_buffer.o http_parse.o http_request.o
	gcc $(CFLAGS) main.o event.o http_epoll.o http_buffer.o http_parse.o http_request.o -g -o server
http_epoll.o: http_epoll.c http_epoll.h event_base.h 
	gcc -c $(CFLAGS) http_epoll.c -g -o http_epoll.o
event.o: http_epoll.h event.h event.c list.h event_base.h http_epoll.h
	gcc -c $(CFLAGS) event.c -g -o event.o
http_buffer.o: http_buffer.c http_buffer.h
	gcc -c $(CFLAGS) http_buffer.c -o http_buffer.o
http_request.o: http_request.c http_request.h http_buffer.h Define_Macro.h
	gcc -c $(CFLAGS) http_request.c -o http_request.o
http_parse.o: http_parse.h http_request.h http_buffer.h
	gcc -c $(CFLAGS) http_parse.c -o http_parse.o
main.o: main.c event.h http_buffer.h http_request.h
	gcc -c $(CFLAGS) main.c -g -o main.o
clean:
	rm -fr *.o
	rm -fr server
	rm -fr core.*
