/* For FreeBSD */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/cdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/disklabel.h>

int main(int argc, char *argv[])
{
	int a, fd;
	for(a=1; a<=argc; a++) { 
		fd = open(argv[a], O_DIRECT | O_NONBLOCK);
		if(fd == -1) continue;
		ioctl(fd, CDIOCEJECT);
	};
}
