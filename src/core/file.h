#ifndef HTTP_FILE_H_INCLUDED
#define HTTP_FILE_H_INCLUDED
#include <sys/types.h>

int lock_file(char *pathname);

int unlock_file(char *pathname);

ssize_t writen(int fd, const void *vptr, size_t n);


ssize_t readn(int fd, void *vptr, size_t n);
#endif
