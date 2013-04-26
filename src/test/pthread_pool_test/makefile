pool: pool.o
	gcc -pthread -g pool.o -o  pool
pool.o: list.h pool.c
	gcc -c -g -pthread pool.c -o  pool.o

clean:
	rm -fr *.o
	rm -fr pool
	rm -fr core.*
