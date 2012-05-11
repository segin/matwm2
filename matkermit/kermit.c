#include "kpacket.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

char kpacket[KPACKET_MAXLEN];
int kfd, seq;

int kermit_send(int type, char *data, int len) {
	len = kpacket_fill(kpacket, seq, type, data, len);
	return write(kfd, kpacket, len);
}

void kermit_recv(void) {
	
}

int main(int argc, char *argv[]) {
	int fd, l;
	fd = open("/dev/ttyUSB0", O_RDWR); /* TODO let user specify & error check */
	l = kpacket_fill(kpacket, 0, KPACKET_TYPE_GEN, "D", 1);
	write(fd, kpacket, l);
	read(fd, kpacket, 4);
	fprintf(stderr, "%c\n", kpacket[3]);
	l = kpacket_fill(kpacket, kpacket[2], KPACKET_TYPE_ACK, "", 0);
	write(fd, kpacket, l);
	while (kpacket[0] != 01)
		read(fd, kpacket, 1);
	read(fd, kpacket, 4);
	fprintf(stderr, "%c\n", kpacket[2]);
	return 0;
}

