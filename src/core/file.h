#ifndef HTTP_FILE_H_INCLUDED
#define HTTP_FILE_H_INCLUDED
unsigned int ReadLine(int filefd, char *buff);
unsigned int ReadCounts(int filefd, char *buff, unsigned int count);

#endif