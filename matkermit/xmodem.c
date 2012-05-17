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
 
char hpxm_buf[1+2+0xFF+1]; /* set max length wisely */
/* CMD + 2 bytes length + argument max length + CRC byte */

int hpxm_cmd(ioh_t *port, char cmd, char *arg) {
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

int hpxm_readdir(ioh_t *port) {
	ioh_t *dirdata = mmemopen(0);
	while (1) {
		if (mfpoll(port, MPOLL_IN, 50) <= 0)
			break;
		mprint("q\n");
		mfxfer(dirdata, port, 2048);
	}
	mfwrite(port, "\06", 1);
	mfprintf(mstderr, "len: %d\n", mmemlen(dirdata));
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
	hpxm_cmd(port, 'L', NULL);
	hpxm_readdir(port);
/*	usleep(200000);
	unsigned char data[0xFFFF];
	int crc = 0;
	int i, l = read(fd, data, 0xFFFF);
	for (i = 2; i < l - 1; ++i)
		crc += data[i];
	mfprintf(mstderr, "\tread bytes:\t%Xh\n\tread length:\t%Xh\n\tcrc guess:\t%Xh\n\tcrc read:\t%2Xh\n", l, (data[0] << 8) | data[1], crc & 0xFF, (unsigned int) data[i]);
	hpxm_cmd(fd, 0x06, NULL);
	for (i = 2; i < l;) {
		mfwrite(mstderr, data+i+1, data[i]);
		unsigned char *p = data+i+1+data[i];
		mfprintf(mstderr, "\t%X %X %X %X %X %X %X\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
		i += data[i]+8;
	}*/
	return EXIT_SUCCESS;
}
