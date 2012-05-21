#include "io.h"
#include <sys/types.h> /* open() */
#include <sys/stat.h>  /* open() */
#include <fcntl.h>     /* open() */
#include <unistd.h>    /* read(), write() */
#include <stdlib.h>    /* NULL, EXIT_FAILURE, EXIT_SUCCESS */
#include <string.h>    /* strlen() */
#include <poll.h>      /* poll() */

/* NOTES
 *
 * When HP calc is in XSERV mode, there are the following commands:
 *   G <filename>        request file download (reply is ACK (06), then send ascii D to get data)
 *   P <filename>        initiate file upload
 *   L                   list files in current directory
 *   l                   list all files in calculator
 *   E <command>         execute command  -- seems we can only run command after first one finished (which is after receiving ACK)
 *   M                   get free memory
 *   V                   get version string
 *   Q                   quit server
 *
 * The command letters are to be sent simply as ASCII character.
 * Arguments take the form of:
 *   HHLL<data>CC
 * Where HH is high byte and LL is low byte of the length.
 * <data> is data of given length, and CC is the sum of those bytes.
 * ... or are my sources wrong and is HH always 0?
 *
 * HP uses a custom NAK, the ASCII character D.
 * And the receiving side is to send this char first time it wants data.
 *
 * Directory listing response as guessed by me (each char a nybble):
 * SSSS<data>CC   for the whole, where SSSS is size (little endian, excluding itself and CRC), and CC is CRC
 *    CRC is the sum of all bytes i think (i probly wrong, confusing results before)
 * LL<name>TTTTSSSSSS????   for individual files, where LL is filename length
 *    TT is type and SSSSSS is file size (big endian, in nyblles, excluding the 8 byte header)
 *    maybe ???? is CRC for the file? (sum1 on usenet says it is CRC of some kind)
 *    seems a \0 (NUL) marks the end of a subdirectory
 *
 * Citing usenet post on the format of directory listings:
 * I do not remember exactly the format for the directory listing, but I think
 * it's of the form:
 * 2 nib size name
 * n bytes: file name
 * 4 nib: prolog
 * 6 nib: file size
 * 4 nib: crc
 * file name size of 0 marks the end of the list
 * for the full file listing, after an object of type directory, the same
 * structure restarts, with a 0 lenght name to end the directory.
 */
 
/* time to wait before asking for data after initiating transfer
 * slacking around a little helps speed up transfers, ironically
 * too short delay, and our first request for data gets lost
 * this value is in microseconds
 * 75000 is about the shortest i could get it without any packet loss
 */
#define GET_PREDELAY 75000

/* time before giving up on expected data to arrive in milliseconds
 * this may be reasonably short, for we will retry a few times
 * (on retry re-sending our request etc)
 * too short is pointless cause we'd give up before anything happens
 * too long and we will get stuck for long time if something goes wrong
 * 250ms seems about right here (it's a fair bit more than required)
 */
#define POLL_DELAY 250

unsigned char hpxm_buf[1+2+0xFF+1]; /* set max length wisely */
/* CMD + 2 bytes length + argument max length + CRC byte */
/* that's also is bigger than the file packets (133 bytes) */

int pollread(ioh_t *h, char *dst, int len) {
	struct pollfd pfd = { .fd = mfdgetfd(h), .events = POLLIN };
	long int pos = 0;
	while (1) {
		if (poll(&pfd, 1, POLL_DELAY) <= 0)
			return pos;
		pos += mfread(h, dst + pos, len - pos);
		if (pos == len)
			return pos;
	}
}

int hpxm_send(ioh_t *port, char cmd, char *arg) {
	int len = 1;
	hpxm_buf[0] = cmd;
	if (arg != NULL) {
		int i, crc = 0, alen = strlen(arg);
		if (alen > sizeof(hpxm_buf) - 4)
			return -1;
		len += 3 + alen;
		hpxm_buf[1] = 0; /* this might argument length high byte, not sure yet */
		hpxm_buf[2] = alen;
		for (i = 0; i < alen; ++i)
			crc += (hpxm_buf[i+3] = arg[i]);
		hpxm_buf[i+3] = crc;
	}
	return mfwrite(port, hpxm_buf, len);
}

int hpxm_recv(ioh_t *port, char **dst) {
	ioh_t *dirdata = mmemopen(0);
	long int i, len, cur;
	int s = 0, tries = 10;
	unsigned char *r;

	start:
	if (pollread(port, hpxm_buf, 2) < 2)
		return -1;
	i = len = ((hpxm_buf[0] << 8) | hpxm_buf[1]) + 1;
	while (1) {
		if (i > sizeof(hpxm_buf)) {
			cur = sizeof(hpxm_buf);
		} else cur = i;
		i -= cur;
		if (pollread(port, hpxm_buf, cur) < cur)
			return -1;
		mfwrite(dirdata, hpxm_buf, cur);
		if (i == 0)
			break;
	}

	r = mmemget(dirdata);
	if (r == NULL)
		return -1;
	for (i = 0; i < len - 1; ++i)
		s += r[i];
	if ((s & 0xFF) != r[i]) {
		if (--tries == 0)
			return -1;
		mfprintf(mstderr, "checksum mismatch, retrying [%d]\n", tries);
		mftrunc(dirdata, 0);
		goto start;
	}

	mfwrite(port, "\06", 1);
	*dst = r;
	return len - 1;
}

/* for L, l, M and V */
int hpxm_ireq(ioh_t *port, char cmd, char **resp) {
	int l, tries = 10; /* that's how many times calc will try */
	start:
	hpxm_send(port, cmd, NULL);
	l = hpxm_recv(port, resp);
	if (l < 0) {
		if (--tries > 0) {
			mfprintf(mstderr, "incomplete or no response, retrying [%d]\n", tries);
			goto start;
		} else return -1;
	}
	return l;
}

/* for G, P, E and Q */
int hpxm_creq(ioh_t *port, char cmd, char *arg) {
	int l, tries = 10;
	char c;
	start:
	hpxm_send(port, cmd, arg);
	while (pollread(port, &c, 1) > 0) {
		if (c == 6)
			return 1;
	}
	if (--tries) {
		mfprintf(mstderr, "no ACK, retrying [%d]\n", tries);
		goto start;
	}
	return -1;
}

int hpxm_get(ioh_t *port, ioh_t *dst, char *filename) {
	int l, len = 0;
	if (hpxm_creq(port, 'G', filename) < 0) {
		mfprintf(mstderr, "failed to initiate tranfer (no ACK)\n");
		return -1;
	}
	usleep(GET_PREDELAY);
	mfwrite(port, "D", 1); /* D for data!... or something */
	while (1) {
		l = pollread(port, hpxm_buf, 133);
		if (l < 133) {
			mfprintf(mstderr, "partial %d\n", l);
			if (l > 0 && hpxm_buf[0] == 4) { /* EOT */
				mfwrite(port, "\06", 1);
				return len;
			}
		} else {
			mfprintf(mstderr, "got packet\n");
			mfwrite(port, "\06", 1);
		}
	}
}

int main(int argc, char *argv[]) {
	ioh_t *port;
	mstdio_init();
	if (argc < 2) {
		mfprintf(mstderr, "error: too few arguments\n");
		return EXIT_FAILURE;
	}
	if ((port = mfopen(argv[1], MFM_RW | MFM_APPEND)) == NULL) {
		mfprintf(mstderr, "error: failed to open port %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	port->options |= MFO_DIRECT;
	{
		char *resp;
		int l;
		/*l = hpxm_ireq(port, 'V', &resp);
		if (l > 0) {
			mfprintf(mstderr, "xserv version:\t%S\n", resp, l);
		} else mfprintf(mstderr, "version receive fail\n");
		l = hpxm_ireq(port, 'M', &resp);
		if (l > 0) {
			mfprintf(mstderr, "free memory:\t%S\n", resp, l);
		} else mfprintf(mstderr, "free mem receive fail\n");*/
		/*if (hpxm_creq(port, 'E', "440 0.05 BEEP 880 0.05 BEEP 1760 0.05 BEEP") <= 0)
			mfprintf(mstderr, "command fail\n");*/
		hpxm_get(port, NULL, "HX2");
	}
/*	usleep(200000);
	unsigned char data[0xFFFF];
	int crc = 0;
	int i, l = read(fd, data, 0xFFFF);
	for (i = 2; i < l - 1; ++i)
		crc += data[i];
	mfprintf(mstderr, "\tread bytes:\t%Xh\n\tread length:\t%Xh\n\tcrc guess:\t%Xh\n\tcrc read:\t%2Xh\n", l, (data[0] << 8) | data[1], crc & 0xFF, (unsigned int) data[i]);
	hpxm_cmd(fd, 0x06, NULL);
	*/
	/*char *resp;
	int i, l = hpxm_ireq(port, 'L', &resp);
	for (i = 0; i < l;) {
		mfwrite(mstderr, resp+i+1, resp[i]);
		unsigned char *p = resp+i+1+resp[i];
		mfprintf(mstderr, "\t%X %X %X %X %X %X %X\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
		i += resp[i]+8;
	}*/
	return EXIT_SUCCESS;
}
