/* Various implementations of dprintf() 
 * Win32 version only writes to sockets, which are not file descriptors.
 * Whole thing's a bloody collection of hacks anyways.
 * I suppose something similar will be needed for OS/2... 
 */

#include "dprintf.h"

int dprintf_wrapper_unix(int fd, char *fmt, ...) 
{
	register int ret;
	FILE *fds;
	va_list ap;
	va_start(ap, fmt);
	fd = dup(fd);
	if((fds = fdopen(fd, "w")) == NULL) return(-1);
	ret = vfprintf(fds, fmt, ap);
	fclose(fds);
	va_end(ap);
	return(ret);
}

int dprintf_wrapper_win32(SOCKET fd, char *fmt, ...)
{
#ifdef __WIN32__
	int fdx = _open_osfhandle(fd, 0);
	register int ret;
	FILE *fds;
	va_list ap;
	va_start(ap, fmt);
	fdx = dup(fdx);
	if((fds = fdopen(fdx, "w")) == NULL) return(-1);
	ret = vfprintf(fds, fmt, ap);
	fclose(fds);
	va_end(ap);
	return(ret);
#endif /* __WIN32__ */
}


