#include "file.h"
#include <stdio.h>
#include <stdlib.h>
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
