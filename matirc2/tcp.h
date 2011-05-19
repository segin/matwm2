#ifndef __TCP_H__
#define __TCP_H__

#ifndef __WIN32__
typedef unsigned int SOCKET;
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif /* !__WIN32__ */

extern char *tcp_error;

SOCKET tcp_connect(char *hostname, char *servname);

#endif /* __TCP_H__ */
