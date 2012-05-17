#include "kpacket.h"
#include "io.h"
#include <sys/types.h> /* open() */
#include <sys/stat.h>  /* open() */
#include <fcntl.h>     /* open() */
#include <unistd.h>    /* read(), write() */
#include <stdlib.h>    /* EXIT_FAILURE, EXIT_SUCCESS */
#include <string.h>    /* strlen() */

#define RETRY_MAX 3

/* TODO
 *   time out when no response
 *
 * NOTES
 *   hp-50g rejects packets if we send too fast after an earlier sequence
 *   best delay seems to be 19000-20000 microseconds
 *   useful calc commands: HOME, UPDIR, EVAL (EVAL makes directory current)
 *   advanced reference says Xmodem can give calcs memory (does includes flags etc?)
 */

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
 *   -2: max retries
 * notes
 *   data field is not decoded by this function
 *   will hang until either a packet is received or transmission broken
 */
int kermit_recv(void) {
	int len, nretry = 0;
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
			if (++nretry > RETRY_MAX)
				return -2;
			kermit_send(KPACKET_TYPE_NAK, NULL, 0);
			mfprintf(mstderr, "checksum failed, retry #%d\n", nretry);
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
 *   dst: destination io handle (NULL is safe if want no output)
 * return value
 *   1: success
 *   -1: received NAK
 *   -2: remote error
 *   -3: kermit_recv fail
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
			case KPACKET_TYPE_ACK:
				return 1;
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
 *   does the same as kermit_send, and then waits for a file or ACK
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
	if (argc < 3) {
		mfprintf(mstderr,
			"***********************************************************\n"
			"* matkermit - kermit client especially for hp calculators *\n"
			"***********************************************************\n"
			"\nsynopsis\n  %s <port> <command> [<arguments>]\n"
			"\ncommands\n"
			"  d                  directory listing\n"
			"  c <directory>      change working directory\n"
			"  g <filename>       get file\n"
			"  p <filename>       upload file (unimplemented atm)\n"
			"  x <command>        run command on calculator\n"
			"\nnotes\n"
			"  messages are sent to stderr (always)\n"
			"  downloaded files are sent to stdout\n"
			"  calculator must be in server mode, rshift(hold)-rightarrow activates that\n"
			"  also you need it to be set to do ASCII transfers (flag 35 on hp-50g)\n"
			"  you might have to be root to use /dev/tty things\n"
			"\n"
			/*"  this is alpha state software, it mite not work well\n"
			"  i provide no warranty, not even if it kills you (tho i would feel sorry)\n"
			"  synopsis etc. may will change, so don't depend on their consistency\n" */
			"  suggestions and comments -> sic_zer0@hotmail.com, my name is mat\n"
			/*"  a catchier name for the program is one suggestion i am looking for\n"*/
			/* above commented cause not fit on 80x25 terminal then */
			,
			argc ? argv[0] : "matkermit");
		return EXIT_SUCCESS;
	}
	if ((kfd = open(argv[1], O_RDWR | O_APPEND)) < 0) {
		mfprintf(mstderr, "error: failed to open port %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	switch (*argv[2]) {
		case 'd':
		case 'D':
			kermit_req(mstdout, KPACKET_TYPE_GEN, "D", 1);
			break;
		case 'g':
		case 'G':
			if (argc < 4) {
				mfprintf(mstderr, "need more argument\n");
				return EXIT_FAILURE;
			}
			kermit_req(mstdout, KPACKET_TYPE_RECV, argv[3], strlen(argv[3]));
			break;
		case 'x':
		case 'X':
			if (argc < 4) {
				mfprintf(mstderr, "need more argument\n");
				return EXIT_FAILURE;
			}
			kermit_req(mstderr, KPACKET_TYPE_CMD, argv[3], strlen(argv[3]));
			mfprint(mstderr, "\n"); /* "Empty Stack" comes without newline */
			break;
		default:
			mfprintf(mstderr, "invalid command, run without arguments for help\n");
	}
	usleep(20000);
	//kermit_req(mstderr, KPACKET_TYPE_GEN, "I", 1);
	return EXIT_SUCCESS;
}
