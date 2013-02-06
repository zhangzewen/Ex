#include "io.h"
#include "http_error.h"

#include <sys/ipc.h>

#include <sys/mman.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

void *Calloc(size_t n,size_t size)
{
	void *ptr;
	if ( (ptr = calloc(n ,size)) == NULL)
		error_quit("calloc error");
	return ptr;
}

void Close(int fd)
{
  if (close(fd) == -1)
		error_sys("Can not close!\n");	
}

void Dup2(int fd1, int fd2)
{
  if (dup2(fd1, fd2) == -1)
		error_sys("dup2 error\n");	
}

int Fcntl(int fd, int cmd, void *arg)
{
  int n;

  if ( (n = fcntl(fd, cmd, arg)) == -1)
		error_sys("fcntl error\n");
  return(n);
}

void * Malloc(size_t size)
{
  void  *ptr;

  if ( (ptr = malloc(size)) == NULL)
		error_quit("malloc error\n");
  return(ptr);
}
void Fstat(int fd, struct stat *ptr)
{
  if (fstat(fd, ptr) == -1)
		error_sys("fstat error\n");
}

off_t Lseek(int fd, off_t offset, int whence)
{
  off_t pos;

  if ( (pos = lseek(fd, offset, whence)) == (off_t) -1)
		error_quit("lseek error\n");
  return(pos);
}

 void * Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  void  *ptr;

  if ( (ptr = mmap(addr, len, prot, flags, fd, offset)) == MAP_FAILED)
		error_sys("mmap error\n");	
  return(ptr);
}

void Munmap(void *addr, size_t len)
{
  if (munmap(addr, len) == -1)
		error_sys("munmap error");
}


int Open(const char *pathname, int oflag, ...)
{
  int   fd;
  va_list ap;
  mode_t  mode;

  if (oflag & O_CREAT) {
    va_start(ap, oflag);    /* init ap to final named argument */
    mode = va_arg(ap, mode_t);
    if ( (fd = open(pathname, oflag, mode)) == -1)
			error_quit("open error");
    va_end(ap);
  } else {
    if ( (fd = open(pathname, oflag)) == -1)
			error_quit("open error");
  }
  return(fd);
}
#if 0
int Ioctl(int fd, int request, void *arg)
{
  int   n;
  if ( (n = ioctl(fd, request, arg)) == -1)
    err_sys("ioctl error");
  return(n);  /* streamio of I_LIST returns value */
}
#endif
int Getopt(int argc, char *const *argv, const char *str)
{
  int   opt;

  if ( ( opt = getopt(argc, argv, str)) == '?')
    exit(1);    /* getopt() has already written to stderr */
  return(opt);
}

key_t Ftok(const char *pathname, int id)
{
  key_t key;

  if ( (key = ftok(pathname, id)) == -1)
		error_sys("ftok error");
  return(key);
}
void Ftruncate(int fd, off_t length)
{
  if (ftruncate(fd, length) == -1)
		error_sys("ftruncate error");
}



long Sysconf(int name)
{
  long  val;

  errno = 0;    /* in case sysconf() does not change this */
  if ( (val = sysconf(name)) == -1) {
    if (errno != 0)
		{}
    else
		{}
  }
  return(val);
}
#if 0
void set_file_flag(int fd, int flags)
{
	int old_flag;
	
	if ((old_flag = fcntl(fd, F_GETFL, 0)) < 0) {
		perror("fcntl GETFL error");
	}
	
	old_flag |= flags;
	
	if((val = fcntl(fd, F_SETFL, old_flag)) < 0) {
		perror("fcntl SETFL error");
	}
}

void clear_file_flag(int fd, int flags)
{
	int old_flag;
	int val;
	
	if ((val = fcntl(fd, F_GETFL, 0)) < 0) {
		perror("fcntl GETFL error");
	}
	
	old_flag &= ~flags;
	
	if((val = fcntl(fd, F_SETFL, old_flag)) < 0) {
		perror("fcntl SETFL error");
	}
}

#endif
