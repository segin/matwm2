/* sys/exit.c: sys_exit C syscall 
 * Written by Kirn Gill <segin2005@gmail.com>
 */
 
#include <sys/syscall.h>

void _Exit(int val)
{
	_exit(val);
}

void _exit(int val)
{
	syscall(sys_exit, val);
}
