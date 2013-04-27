#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "http_pthread.h"
void *do_something(void *arg) {
    int n = *(int *)arg;
    printf("task #%d started\n", n);
    printf("task #%d finished\n", n);
		return NULL;
}

int main(int argc, char *argv[])
{
	thread_t thread;
	int a = 10;
	thread = thread_create(NULL,do_something, (void *)(&a));
	while(1){
	}
	return 0;
}
