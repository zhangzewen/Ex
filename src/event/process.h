#ifndef _HTTP_PROCESS_H_INCLUDED_
#define _HTTP_PROCESS_H_INCLUDED_
pid_t Fork(void)
{
	pid_t pid;
	if((pid = fork()) < 0) {
		perror("fork error");
		return -1;
	}
	return pid;
}
#endif
