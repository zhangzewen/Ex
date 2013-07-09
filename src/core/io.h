#ifndef __HTTP_IO_H__
#define __HTTP_IO_H__


int set_file_flag(int fd, int flags);

int clear_file_flag(int fd, int flags);

int set_fd_nonblock(int fd);
int set_fd_block(int fd);
#endif
