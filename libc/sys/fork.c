/* sys/fork.c: sys_fork C syscall */

#include <sys/syscall.h>

int fork(void) 
{
	return(syscall(sys_fork));
}
