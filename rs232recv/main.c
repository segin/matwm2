/*
 * quick hack to receive data by rs-232
 */


/* open() */
#include <sys/types.h> /* ftruncate() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> /* read(), close(), tcsetattr(), ftruncate(), usleep() */
#include <termios.h> /* tcsetattr() */
#include <stdio.h> /* perror() */
#include <stdlib.h> /* EXIT_FAILURE */
#include <string.h> /* memcpy(), strcmp() */
/* ioctl(), TIOCMGET, TIOCMSET */
#include <sys/ioctl.h>
#include <termios.h>

#define DEVICE_DEFAULT "/dev/ttyUSB0"

int softflow = 0;
int speed = B300;
struct termios oldattr;

int openterm(char *filename) {
	struct termios tattr;
	int fd = open(filename, O_RDWR | O_NOCTTY);
	if (fd < 0)
		return fd;
	tcflush(fd, TCIOFLUSH);
	tcgetattr(fd, &oldattr);
	memcpy((void *) &tattr, (void *) &oldattr, sizeof(struct termios));
	tattr.c_iflag = IGNBRK;
	tattr.c_cflag = (tattr.c_cflag & ~CSIZE) | CS8;
	tattr.c_cflag |= CLOCAL | CREAD;
	tattr.c_lflag = 0;
	tattr.c_oflag = 0;
	tattr.c_cc[VTIME] = 10; /* 1 sec timeout for noncanonical read */
	tattr.c_cc[VMIN] = 1; /* 1 characters minimum for noncanonical read */
	if (softflow) {
		tattr.c_cflag &= ~CRTSCTS;
		tattr.c_iflag |= IXON | IXOFF;
	} else {
		tattr.c_cflag |= CRTSCTS;
		tattr.c_iflag &= ~(IXON|IXOFF);
	}
	cfsetspeed(&tattr, speed);
	if (tcsetattr(fd, TCSANOW, &tattr) < 0) {
		close(fd);
		return -1;
	}
	fprintf(stderr, "Successfully opened serial device '%s'.\n", filename);
	return fd;
}

void closeterm(int fd) {
	tcsetattr(fd, TCSANOW, &oldattr);
	close(fd);
}

int getfile(int fd) {
	char buf[512];
	int n;
	{
		int i, ofd, flen = 0;
		char filename[128] = "capture.txt";
		fprintf(stderr, "\tReceiving '%s'\n", filename);
		ofd = open(filename, O_RDWR | O_CREAT, 0644);
		if (ofd < 0)
			goto nocreat;
		while ((n = read(fd, buf, sizeof(buf))) > 0) {
			write(ofd, buf, n);
			flen += n;
		}
		fprintf(stderr, "\tFile possibly incomplete!\n");
		stop:
			flen += i;
			ftruncate(ofd, flen);
			close(ofd);
			fprintf(stderr, "\tDone.\n");
			return 1;
		nocreat:
			fprintf(stderr, "\tError opening file '%s'.\n", filename);
			return 1;
	}
	die:
	fprintf(stderr, "\tError receiving file.\n");
	return 1;
}

int main(int argc, char *argv[]) {
	int fd, i;
	char *device = DEVICE_DEFAULT;

	++argv, --argc;
	for (i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "-x") == 0)
			softflow = 1;
	}

	fd = openterm(device);
	if (fd < 0) {
		fprintf(stderr, "Error opening device '%s'.\n", device);
		return EXIT_FAILURE;
	}
	do {
		usleep(100000); /* 100mS */
		tcflush(fd, TCIOFLUSH); /* so any excess is flushed */
		fprintf(stderr, "[ready for data]\n");
	} while(getfile(fd));
	closeterm(fd);
	return 0;
}

