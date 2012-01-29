/* sys/write.c: sys_Write C syscall */

#include <unistd.h>
#include <sys/syscall.h>

ssize_t write(int fd, void *buf, size_t len)
{
	return((ssize_t) syscall(sys_write, fd, buf, len));
}
