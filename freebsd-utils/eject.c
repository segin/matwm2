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

#define ejerror(msg) fprintf(stderr, msg " %s: %s\n", argv[a], strerror(errno));

int main(int argc, char *argv[])
{
	int a, fd;
	for(a = 1; a <= argc; a++) { 
		if((fd = open(argv[a], O_DIRECT | O_NONBLOCK)) != -1) {
			if(ioctl(fd, CDIOCEJECT) == -1) {
				ejerror("ioctl(CDIOCEJECT) failed for");
			} 
			close(fd);
		} else { 
			ejerror("Cannot eject");
		}
	};
}
