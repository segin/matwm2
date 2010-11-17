/* For FreeBSD 
 * 
 * vi: ts=4 sw=4 ai=1
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/cdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/disklabel.h>

int main(int argc, char *argv[])
{
	int a, fd;
	for(a = 1; a <= argc; a++) { 
		fd = open(argv[a], O_DIRECT | O_NONBLOCK);
		if(fd != -1) { 
			ioctl(fd, CDIOCEJECT);
			close(fd);
		} else { 
			fprintf(stderr, "Cannot eject %s: %s\n", argv[a], strerror(errno));
		}
	};
}
