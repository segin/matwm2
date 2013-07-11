/* open() */
#include <sys/types.h> /* ftruncate() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> /* read(), close(), tcsetattr(), ftruncate(), usleep() */
#include <termios.h> /* tcsetattr() */
#include <stdio.h> /* perror() */
#include <stdlib.h> /* EXIT_FAILURE */
#include <string.h> /* memcpy() */
#include <time.h> /* time() */
/* ioctl(), TIOCMGET, TIOCMSET */
#include <sys/ioctl.h>
#include <termios.h>

#define DEVICE_DEFAULT "/dev/ttyUSB0"

int softflow = 1;
int speed = B19200;
struct termios oldattr;

int openterm(char *filename) {
	struct termios tattr;
	int mcs, fd = open(filename, O_RDWR | O_NOCTTY);
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
	tattr.c_cc[VTIME] = 10; /* 1 second timeout for noncanonical read */
	tattr.c_cc[VMIN] = 0; /* 0 characters minimum for noncanonical read */
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
	char buf[512]; /* must be > 3 or segfault (look below for why) */
	int n;
	while (1) {
		if ((n = read(fd, buf, 1)) < 0)
			goto die;
		if (buf[0] != 0x1B || n < 1)
			continue;
		if (read(fd, buf, 1) < 0)
			goto die;
		if (buf[0] != 0x2E)
			continue;
		if (read(fd, buf, 1) < 0)
			goto die;
		if (buf[0] != 0x59)
			continue;
		break;
	}
	{
		int i, ofd, flen = 0;
		char filename[128] = "capture.txt";
		time_t t = time(NULL);
		snprintf(filename, sizeof(filename), "capture-%d.txt", t);
		fprintf(stderr, "\tReceiving '%s'\n", filename);
		ofd = open(filename, O_RDWR | O_CREAT, 644);
		if (ofd < 0)
			goto nocreat;
		while ((n = read(fd, buf, sizeof(buf))) > 0) {
			write(ofd, buf, n);
			for (i = 0; i < n; ++i) {
				if (buf[i] == 0x1B)
					goto stop;
			}
			flen += n;
		}
		fprintf(stderr, "\tFile possibly incomplete!\n");
		stop:
			flen += i;
			ftruncate(ofd, flen);
			close(ofd);
			fprintf(stderr, "\tDone.\n", filename);
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
	int fd;
	char *device = DEVICE_DEFAULT;

	fd = openterm(device);
	if (fd < 0) {
		perror("error");
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
