/*
 * Serial data starts with: 1B 2E 59
 * And it ends with: 1B 2E 5A
 *
 * Inbetween is sequence of commands (listed below).
 * Commands are terminated with ';', arguments follow the command (without separator).
 * Multiple argumentsa re separated by ','.
 * The argument for SR is given as floating point number.
 * String argument ends with \x03 (no ;).
 *
 * IN                       initialize
 * SC xmin,xmax,ymin,ymax   scale
 * SR w,h                   set font size
 * SP x                     select pen
 * PA x,y                   plot absolute
 * PD                       pen down
 * PU                       pen up
 * LB str                   label
 */

int iscmd(char **data, char *cmd) {
	int i;
	for (i = 0; cmd[i] != 0 && cmd[i] == (*data)[i]; ++i);
	if (cmd[i] == 0) {
		*data += i;
		return 1;
	}
	return 0;
}

int getnumlen;

double getnum(char **src) {
	double num = 0;
	int depth = 0;
	int dot = 0;
	getnumlen = 0;
	while ((**src >= '0' && **src <= '9') || **src == '.') {
		if (**src == '.') {
			dot = 1;
			++*src;
			continue;
		}
		num *= 10;
		num += **src - '0';
		if (dot)
			++depth;
		++*src, ++getnumlen;
	}
	for (; depth > 0; --depth)
		num /= 10;
	return num;
}

void skipsp(char **data) {
	while (**data == ' ' || **data == '\t' || **data == '\n')
		++*data;
}

#define MAXARGS 16

double args[MAXARGS];
int nargs;

int getargs(char **data, int min) {
	nargs = 0;
	getnumlen = 1;
	while (getnumlen > 0 && nargs < MAXARGS) {
		skipsp(data);
		args[nargs] = getnum(data);
		skipsp(data);
		if (**data == ',')
			++*data;
		skipsp(data);
		++nargs;
	}
	if (nargs < min)
		return 0;
	return 1;
}

#include "io.h"
#include "draw.h"

void hpgl_plot(char *data) {
	int pen = 0;
	int down = 0;
	double x = 0, y = 0;

	draw_init();
	start:
	skipsp(&data);
	if (iscmd(&data, "IN")) {
		goto nextcmd;
	}
	if (iscmd(&data, "SC")) {
		if (!getargs(&data, 4))
			goto toofew;
		/* TODO set the scale */
		goto nextcmd;
	}
	if (iscmd(&data, "SR")) {
		if (!getargs(&data, 2))
			goto toofew;
		/* TODO set font size */
		goto nextcmd;
	}
	if (iscmd(&data, "SP")) {
		if (!getargs(&data, 1))
			goto toofew;
		pen = args[0];
		goto nextcmd;
	}
	if (iscmd(&data, "PA")) {
		if (!getargs(&data, 2))
			goto toofew;
		if (down) {
			draw_line(x, y, args[0], args[1]);
		}
		x = args[0];
		y = args[1];
		goto nextcmd;
	}
	if (iscmd(&data, "PD;PU")) {
		draw_dot(x, y);
		goto nextcmd;
	}
	if (iscmd(&data, "PD")) {
		down = 1;
		goto nextcmd;
	}
	if (iscmd(&data, "PU")) {
		down = 0;
		goto nextcmd;
	}
	if (iscmd(&data, "LB")) {
		char *str = data;
		while (*data != 0x03 && *data != 0)
			++data;
		if (*data == 0) {
			mfprintf(mstderr, "warning: unexpected EOF or 0\n");
			return;
		}
		*data = 0;
		draw_text(x, y, str);
		goto nextcmd;
	}
	if (*data == 0)
		return;
	mfprintf(mstderr, "warning: unknown command\n");
	nextcmd:
		while (*data != ';' && *data != '\n' && *data != 0 && *data != 0x03)
			++data;
		++data;
		if (*data != 0)
			goto start;
		return;
	toofew:
		mfprintf(mstderr, "warning: too few arguments for command\n");
		goto nextcmd;
}

#include <stdlib.h> /* NULL, exit(), EXIT_FAILURE, realloc() */
#include <unistd.h> /* exit() */
#include <stdarg.h>

#define BLOCK 4096

void errexit(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	mfprint(mstderr, "error: ");
	mvafprintf(mstderr, fmt, ap);
	mfprint(mstderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

char *readfile(char *path) {
	int pos = 0, mem = 0, r = 0;
	ioh_t *infd = mstdin;
	char *ret;
	if (path != NULL)
		infd = mfopen(path, MFM_RD);
	if (infd == NULL)
		errexit("failed to open file \"%s\"", path);
	do {
		pos += r;
		if (mem < pos + BLOCK) {
			if (mem + BLOCK < mem)
				errexit("wtf integer overflow");
			mem += BLOCK;
			ret = (char *) realloc((void *) ret, mem + 1); /* + 1 for ending 0 */
			if (ret == NULL)
				errexit("out of memory");
		}
	} while((r = mfread(infd, ret + pos, BLOCK)) > 0);
	ret[pos] = 0;
	if (path != NULL)
		mfclose(infd);
	return ret;
}

int main(int argc, char *argv[]) {
	char *file;
	mstdio_init();
	file = readfile((argc > 1) ? argv[1] : NULL);
	hpgl_plot(file);
	return 0;
}

