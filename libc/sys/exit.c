/* sys/exit.c: sys_exit C syscall */

#include <sys/syscall.h>

void exit(int val)
{
	syscall(sys_exit, val);
}
