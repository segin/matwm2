#include "kpacket.h"
#include <sys/types.h> /* open() */
#include <sys/stat.h>  /* open() */
#include <fcntl.h>     /* open() */
#include <unistd.h>    /* read(), write() */
#include <stdio.h>     /* fprintf() */
#include <stdlib.h>    /* EXIT_FAILURE, EXIT_SUCCESS */

char kpacket[KPACKET_MAXLEN];
int kfd, seq = 0;

int kermit_send(int type, char *data, int len) {
	len = kpacket_fill(kpacket, seq, type, data, len);
	return write(kfd, kpacket, len);
}

void kermit_recv(void) {
	
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "error: too few arguments\n");
		return EXIT_FAILURE;
	}
	if ((kfd = open(argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "error: failed to open port %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	kermit_send(KPACKET_TYPE_CMD, "440 1 BEEP", 10);
	return EXIT_SUCCESS;
}

