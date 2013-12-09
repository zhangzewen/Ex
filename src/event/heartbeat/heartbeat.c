#include "event.h"

#include <stdio.h>
#include <stdlib.h>

extern int s_heartbeat;






int send_event()
{
	//send,开始计时，在未规定的时间内收到正确的s_heartbeat++	
}

int recv_event()
{
	//recv,读包，要是错了，就还回，然后send包，在未规定的时间内没有收到正确的包，s_heartbeat++
}



int heartbeat_process()
{
	
	if (strcmp(current_state, "MASTER") == 0) {
		printf("current_state = %s\n", current_state);
	} else if (strcmp(current_state, "BACKUP") == 0) {
		printf("current_state = %s\n", current_state);
	} else {
		printf("current_state = %s\n", current_state);
	}

	return 0;

	enum {
		SEND_SYN,
		RECV_ACK
	} state;

	state = SEND_SYN;

	while(1) {
		switch (state) {
			case SEND_SYN:
				break;
			case RECV_ACK:
				break;
			default:
				break;
		}
	}
	
}
