#ifndef HTTP_FILE_H_INCLUDED
#define HTTP_FILE_H_INCLUDED
unsigned int readline(int filefd, char *buff);
unsigned int readcounts(int filefd, char *buff, unsigned int count);

#endif
