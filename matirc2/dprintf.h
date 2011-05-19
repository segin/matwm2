#ifndef __DPRINTF_H__
#define __DPRINTF_H__

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(__APPLE__) && defined(__MACH__) 
#define dprintf(...) dprintf_wrapper_unix(__VA_ARGS__);
#endif /* defined(__APPLE__) && defined(__MACH__) */

#ifndef __WIN32__
typedef unsigned int SOCKET;
#else
#define dprintf(...) dprintf_wrapper_win32(__VA_ARGS__);
#include <winsock2.h>
#include <ws2tcpip.h>
#endif /* __WIN32__ */

int dprintf_wrapper_unix(int fd, char *fmt, ...);
int dprintf_wrapper_win32(SOCKET fd, char *fmt, ...);

#endif /* __DPRINTF_H__ */
