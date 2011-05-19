#include "tcp.h"

#include <string.h>
#include <strings.h>
#include <sys/types.h>

char *tcp_error = NULL; /* if tcp_connect() fails it returns -1 and this will be set */

SOCKET tcp_connect(char *hostname, char *servname) {
	SOCKET ret;
	struct addrinfo hints, *addrinfo;
	/* get adress information */
	memset((void *) &hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	ret = getaddrinfo(hostname, servname, &hints, &addrinfo);
	if(ret) {
		tcp_error = (char *) gai_strerror(ret);
		return -1;
	}
	/* create socket */
	if((ret = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol)) == -1) {
		tcp_error = strerror(errno);
		freeaddrinfo(addrinfo);
		return -1;
	}
	/* connect */
	if(connect(ret, addrinfo->ai_addr, addrinfo->ai_addrlen) == -1) {
		tcp_error = strerror(errno);
		freeaddrinfo(addrinfo);
		close(ret);
		return -1;
	}
	/* free address information */
	freeaddrinfo(addrinfo);
	return ret;
}


