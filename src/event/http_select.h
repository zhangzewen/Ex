#ifndef __HTTP_EVENT_SELECT_H_INCLUDED_
#define __HTTP_EVENT_SELECT_H_INCLUDED_

#define FD_SET_ZERO			0x1
#define FD_SET_DELETE		0X2
#define FD_SET_ADD			0X3
#define FD_SET_ISSET 		0X4


int add_event(int fd);
int del_event(int fd);


#endif
