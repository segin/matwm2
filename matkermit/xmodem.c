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
 *
 * HP uses a custom NAK, the ASCII character D.
 * And the receiving side is to send this char first time it wants data.
 *
 * Directory listing response as guessed by me (each char a nybble):
 * SSSS<data>CC   for the whole, where SSSS is size (little endian, excluding itself and CRC), and CC is CRC
 *    CRC is the sum of all bytes i think (i probly wrong, confusing results before)
 * LL<name>TTTTSSSSSS????   for individual files, where LL is filename length
 *    TT is type and SSSSSS is file size (big endian, in nyblles, excluding the 8 byte header)
 *    maybe ???? is CRC for the file? (sum1 on usenet says it is CRC of some kind, it changes when file changes and not when filename changes)
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
	hpxm_buf[0] = cmd;
	if (arg != NULL) {
		int i, crc = 0, alen = strlen(arg);
		if (alen > 0xFFFF)
			return -1;
		hpxm_buf[1] = alen >> 8;
		hpxm_buf[2] = alen & 0xFF;
		if (mfwrite(port, hpxm_buf, 3) < 0)
			return -1;
		if (mfwrite(port, arg, alen) < 0)
			return -1;
		for (i = 0; i < alen; ++i)
			crc += arg[i];
		hpxm_buf[0] = crc;
		if (mfwrite(port, hpxm_buf, 1) < 0)
			return -1;
	} else if (mfwrite(port, hpxm_buf, 1))
			return -1;
	return 1;
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
	int tries = 10;
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

unsigned short crc16tab[] = {
	0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
	0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
	0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
	0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
	0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
	0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
	0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
	0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
	0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
	0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
	0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
	0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
	0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
	0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
	0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
	0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
	0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
	0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
	0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
	0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
	0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
	0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
	0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
	0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
	0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
	0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
	0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
	0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
	0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
	0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
	0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
	0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78,
};

unsigned short crc16(unsigned char *data, int len) {
	unsigned short crc = 0;
	int i;
	for (i = 0; i < len; ++i)
		crc = (crc >> 8) ^ crc16tab[(crc ^ data[i]) & 0xff];
	return crc;
}

int hpxm_get(ioh_t *port, ioh_t *dst, char *filename) {
	int l, len = 0, seq = 1, resend_tries = 10;
	if (hpxm_creq(port, 'G', filename) < 0) {
		mfprintf(mstderr, "failed to initiate tranfer (no ACK)\n");
		return -1;
	}
	usleep(GET_PREDELAY);
	resend:
	mfwrite(port, "D", 1); /* D for data!... or something */
	while (1) {
		l = pollread(port, hpxm_buf, 133);
		if (l < 133) {
			if (l <= 0) {
				if (--resend_tries) {
					mfprintf(mstderr, "no data, asking resend [%d]\n", resend_tries);
					goto resend;
				}
				return -1;
			}
			if (l > 0) {
				int i;
				for (i = 0; i < l; ++i) {
					switch (hpxm_buf[i]) {
						case 0x04: /* EOT */
							mfwrite(port, "\06", 1);
							return len;
						case 0x15: /* CANCEL */
							mfprintf(mstderr, "remote host cancelled\n");
							return -1;
					}
				}
			}
		} else {
			if (hpxm_buf[1] != (~hpxm_buf[2] & 0xFF)) {
				mfprintf(mstderr, "sequence data corrupt, retrying\n");
				mfwrite(port, "D", 1);
				continue;
			}
			if (hpxm_buf[1] > seq) {
				mfprintf(mstderr, "lost packet, cancelling\n");
				mfwrite(port, "\030\030\030", 3); /* three times cancel */
				return -1;
			}
			if (hpxm_buf[1] < seq) {
				mfprintf(mstderr, "out of sequence packet, (re?)sending ACK\n");
				mfwrite(port, "\06", 1);
				continue;
			}
			if ((hpxm_buf[132] | (hpxm_buf[131] << 8)) != crc16(hpxm_buf + 3, l-5)) {
				mfprintf(mstderr, "checksum failed, retrying\n");
				mfwrite(port, "D", 1);
				continue;
			}
			mfwrite(dst, hpxm_buf + 3, l-5);
			++seq;
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
		int i, l;/*

		l = hpxm_ireq(port, 'V', &resp);
		if (l > 0) {
			mfprintf(mstderr, "xserv version:\t%S\n", resp, l);
		} else mfprintf(mstderr, "version receive fail\n");

		l = hpxm_ireq(port, 'M', &resp);
		if (l > 0) {
			mfprintf(mstderr, "free memory:\t%S\n", resp, l);
		} else mfprintf(mstderr, "free mem receive fail\n");
*/
		l = hpxm_ireq(port, 'L', &resp);
		for (i = 0; i < l;) {
			mfprintf(mstderr, "%4X ", crc16(resp+i, resp[i]+6));
			mfwrite(mstderr, resp+i+1, resp[i]);
			unsigned char *p = resp+i+1+resp[i];
			mfprintf(mstderr, "\ttype %2X%2X size %d\t%2X %2X\n", p[0], p[1], p[2] | (p[3] << 8) | (p[4] << 16), p[5], p[6]);
			i += resp[i]+8;
		}

		hpxm_get(port, mstdout, "VX");

		/*if (hpxm_creq(port, 'E', "440 0.05 BEEP 880 0.05 BEEP 1760 0.05 BEEP") <= 0)
			mfprintf(mstderr, "command fail\n");*/
	}

	return EXIT_SUCCESS;
}
