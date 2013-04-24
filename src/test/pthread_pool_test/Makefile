CFLAGS = -g -Wall -Werror -pthread -I ../../core/   \
					                         -I../../event/

Objs = ../../core/http_error.o \
			 ../../event/http_pthread.o

pool: pool.o ${Objs}
	gcc ${CFLAGS} pool.o ${Objs} -o pool

pool.o: pool.c
	gcc -c ${CFLAGS} pool.c -o pool.o
clean:
	rm -fr *.o
	rm -fr pool
