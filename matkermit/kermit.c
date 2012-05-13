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
	len = unchar(kpacket[1]) + 2;
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
	int i, len = unchar(kpacket[1]) + 1;
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
 *   -1: received NAK
 *   -2: remote error
 *   -3: 
 */
int kermit_get(ioh_t *dst) {
	int lastseq = -1;
	while (1) {
		if (!kermit_recv())
			return -3;
		if (lastseq == kpacket[2])
			continue;
		lastseq = kpacket[2];
		switch (kpacket[3]) {
			case KPACKET_TYPE_DATA:
				kermit_decode(dst);
				kermit_send(KPACKET_TYPE_ACK, NULL, 0);
				break;
			case KPACKET_TYPE_EOT:
				kermit_send(KPACKET_TYPE_ACK, NULL, 0);
				return 1;
			case KPACKET_TYPE_ERR:
				mfprint(mstderr, "error: ");
				kermit_decode(mstderr);
				mfprint(mstderr, "\n");
				return -2;
			case KPACKET_TYPE_SEND:
				kseq = 0;
			case KPACKET_TYPE_EOF:
			case KPACKET_TYPE_FILE:
			case KPACKET_TYPE_TEXT:
				kermit_send(KPACKET_TYPE_ACK, NULL, 0);
				break;
			case KPACKET_TYPE_NAK:
				return -1;
			default:
				mfprintf(mstderr, "unknown packet type %c, sending ACK anyway\n", kpacket[3]);
				kermit_send(KPACKET_TYPE_ACK, NULL, 0);
		}
	}
	mfflush(dst);
}

/* kermit_req
 *
 * description
 *   does the same as kermit_send, and then waits for a file
 * input
 *   dst: destination io handle
 *   type: packet type
 *   data: data
 *   len: data length in bytes (not including packet stuff), 0-94
 * return value
 *    1 success
 *   -1 failed sending packet
 * notes
 *   data field is not encoded by this function
 *   seems if we run sequences of this we have to wait 20000 usec
 *    or receive a NAK for the first try
 */
int kermit_req(ioh_t *dst, int type, char *data, int len) {
	kseq = 0;
	do if (kermit_send(type, data, len) <= 0)
		return -1;
	while (kermit_get(dst) == -1); /* -1 is received NAK */
	return 1;
}

int main(int argc, char *argv[]) {
	mstdio_init();
	if (argc < 2) {
		mfprint(mstderr, "error: too few arguments\n");
		return EXIT_FAILURE;
	}
	if ((kfd = open(argv[1], O_RDWR | O_APPEND)) < 0) {
		mfprintf(mstderr, "error: failed to open port %s\n", argv[1]);
		return EXIT_FAILURE;
	}
/*	kermit_req(mstdout, KPACKET_TYPE_CMD, "CLEAR", 5);
	mfprint(mstdout, "\n");*/
	//usleep(20000); /* TODO must figure why it doesn't work without this delay */
	kermit_req(mstdout, KPACKET_TYPE_GEN, "D", 1);
	usleep(20000);
	kermit_req(mstdout, KPACKET_TYPE_RECV, "HELLO", 5);
	return EXIT_SUCCESS;
}
