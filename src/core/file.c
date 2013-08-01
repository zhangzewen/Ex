#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

int lock_file(char *pathname) 
{
  int fd;
  struct flock lock;
  char buf[10], filename[1024];

  memset(filename, '\0', sizeof(filename));
  strncpy(filename, pathname, sizeof(filename) - 1);

  if ((fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
    goto out;

  lock.l_type   = F_WRLCK;
  lock.l_whence = SEEK_SET;   
  lock.l_start  = 0;          
  lock.l_len    = 0;

  if (fcntl(fd, F_SETLK, &lock) == -1)  
    goto err;

  ftruncate(fd, 0);

  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf), "%ld", (long) getpid());
  write(fd, buf, strlen(buf));
  return 0;
err:                          
  close(fd);
out:
  return -1;
}

int unlock_file(char *pathname)
{
	int fd;
	struct flock lock;
	char filename[1024] = {0};
	
	strncpy(filename, pathname, sizeof(filename) - 1);


	if ((fd = open(filename, O_RDWR | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
		goto out;
	}

	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	

	if (fcntl(fd, F_SETLKW, &lock) == -1) {
		goto err;
	}

	return 0;

err:
	close(fd);
out:
	return -1;
}


ssize_t writen(int fd, const void *vptr, size_t n)
{
  size_t    nleft;
  ssize_t   nwritten;
  const char  *ptr;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
      if (nwritten < 0 && errno == EINTR)
        nwritten = 0;   
      else
        return(-1);  
    }   

    nleft -= nwritten;
    ptr   += nwritten;
  }
  return(n);
}


ssize_t readn(int fd, void *vptr, size_t n)
{
  size_t  nleft;
  ssize_t nread;
  char  *ptr;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ( (nread = read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR)
        nread = 0;
      else
        return(-1);
    } else if (nread == 0)
      break;

    nleft -= nread;
    ptr   += nread;
  }
  return(n - nleft);
}
