#include "kpacket.h"
#include "io.h"
#include <sys/types.h> /* open() */
#include <sys/stat.h>  /* open() */
#include <fcntl.h>     /* open() */
#include <unistd.h>    /* read(), write() */
#include <stdlib.h>    /* EXIT_FAILURE, EXIT_SUCCESS */

#define RETRY_MAX 3

/* Absolute maximum length for kermit packet we will use */
/* 4 bytes header + max data (94) + 1 byte checksum + \r\n + NUL = 102 */
#define KPACKET_MAXLEN 102

char kpacket[KPACKET_MAXLEN];
int kfd, kseq = 0;

/* kermit_send
 *
 * description
 *   fills kermit packet and sends it
 * input
 *   type: packet type
 *   data: data
 *   len: data length in bytes (not including packet stuff), 0-94
 * return value
 *   return of write()
 * notes
 *   data field is not encoded by this function
 */
int kermit_send(int type, char *data, int len) {
	int i, s = 0;
	kpacket[0] = 01;
	s += kpacket[1] = tochar(len + 3);
	s += kpacket[2] = tochar(kseq % 64);
	s += kpacket[3] = type;
	for (i = 0; i < len; ++i) {
		s += data[i];
		kpacket[i+4] = data[i];
	}
	i += 4;
	kpacket[i++] = tochar((s + ((s & 192) / 64)) & 63);
	kpacket[i++] = '\r';
	kpacket[i++] = '\n';
	return write(kfd, kpacket, i);
}

/* kermit_recv
 *
 * description
 *   waits for server data and receives exactly one packet
 * return value
 *   1: success
 *   -1: read error
 * notes
 *   data field is not decoded by this function
 *   will hang until either a packet is received or transmission broken
 */
int kermit_recv(void) {
	int len;
	start:
	len = 1;
	while (1) {
		if (read(kfd, kpacket, 1) <= 0)
			return -1;
		if (kpacket[0] == 01)
			break;
	}
	while (1) {
		len += read(kfd, kpacket + len, KPACKET_MAXLEN - len);
		if (len >= 2 && len >= unchar(kpacket[1]) + 2)
			break;
	}
	mfprintf(mstderr, "olen: %d\n", len);
	len = unchar(kpacket[1]) + 2;
	mfprintf(mstderr, "len: %d\n", len);
	mfprintf(mstderr, "got packet type %c\n", kpacket[3]);
	{
		int i, s = 0;
		for (i = 1; i < len - 1; ++i)
			s += kpacket[i];
		if (kpacket[len-1] != tochar((s + ((s & 192) / 64)) & 63)) {
			kermit_send(KPACKET_TYPE_NAK, NULL, 0);
			mfprint(mstderr, "checksum failed, retrying\n");
			goto start;
		}
	}
	kseq = unchar(kpacket[2]);
	return len;
}

/* kermit_decode
 *
 * description
 *   decodes data field of kermit packet
 * input
 *   dst: destination io handle
 */
void kermit_decode(ioh_t *dst) {
	int i, len = unchar(kpacket[1]) - 1;
	for (i = 4; i < len; ++i) {
		if (kpacket[i] == '#')
			mfprintf(dst, "%c", ctl(kpacket[++i]));
		else mfwrite(dst, kpacket + i, 1);
	}
}

/* kermit_get
 *
 * description
 *   retrieves file (after transfer has been initiated some way)
 * input
 *   dst: destination io handle
 * return value
 *   1: success
 *   -1: error
 */
int kermit_get(ioh_t *dst) {
	while (1) {
		if (!kermit_recv())
			return -1;
		switch (kpacket[3]) {
			case 'D':
				kermit_decode(dst);
				kermit_send(KPACKET_TYPE_ACK, NULL, 0);
				break;
			default:
				mfprintf(mstderr, "unknown packet, sending ACK anyway\n");
				kermit_send(KPACKET_TYPE_ACK, NULL, 0);
		}
	}
}

int main(int argc, char *argv[]) {
	mstdio_init();
	if (argc < 2) {
		mfprint(mstderr, "error: too few arguments\n");
		return EXIT_FAILURE;
	}
	if ((kfd = open(argv[1], O_RDWR)) < 0) {
		mfprintf(mstderr, "error: failed to open port %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	kermit_send(KPACKET_TYPE_CMD, "CLEAR", 5);
	kermit_get(mstdout);
	return EXIT_SUCCESS;
}
