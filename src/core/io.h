#ifndef _HTTP_IO_H_INCLUDED_
#define _HTTP_IO_H_INCLUDED_
#include <sys/ipc.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include "error.h"
void *Calloc(size_t n,size_t size);
void Close(int fd);
void Dup2(int fd1, int fd2);
int Fcntl(int fd, int cmd, void *arg);
void * Malloc(size_t size);
void Fstat(int fd, struct stat *ptr);
off_t Lseek(int fd, off_t offset, int whence);
void * Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void Munmap(void *addr, size_t len);
int Open(const char *pathname, int oflag, ...);
int Ioctl(int fd, int request, void *arg);
int Getopt(int argc, char *const *argv, const char *str);
key_t Ftok(const char *pathname, int id);
void Ftruncate(int fd, off_t length);
long Sysconf(int name);

void set_file_flag(int fd, int flags);
void clear_file_flag(int fd, int flags);
#endif
