CFLAGS = -g -Wall -Werror -pthread -I ../../core/   \
					                         -I../../event/   \
																	 -I../../http/

Objs = ../../core/http_error.o \
			 ../../http/http_parse.o

response: response.o ${Objs}
	gcc ${CFLAGS} response.o ${Objs} -o response

response.o: response.c
	gcc -c ${CFLAGS} response.c -o response.o
clean:
	rm -fr *.o
	rm -fr response
	rm -fr core.*
