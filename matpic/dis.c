#include "arch.h"
#include "str.h" /* skipsp(), skipnl(), alfa[], hexlookup[] */
#include "mem.h"
#include "misc.h" /* flerrexit() */
#include "main.h" /* infile, address */
#include "dis.h"

arr_t dsym = { NULL, 0, 0, 0 };

void ihwarn(char *msg) {
	flwarn(infile, line, msg);
}

void dwarn(char *msg) {
	fawarn(infile, address, msg);
}

int gethnum(char **src) {
	int r, l = hexlookup[(unsigned char) **src];
	if (l == 16) /* really check inbetween, in case we hit terminating 0 */
		return -1;
	r = hexlookup[(unsigned char) *++*src];
	if (l == 16)
		return -1;
	*src += 2;
	return (l << 4) | r;
}

void readihex(char *in) {
	int n, len, crc = 0;
	dsym_t ds;

	arr_new(&dsym, sizeof(dsym_t));
	dstartline:
	--line;
	do {
		skipsp(&in);
		++line;
	} while (skipnl(&in));
	if (*in == 0) {
		ihwarn("no end record");
		return;
	}
	if (*in != ':')
		goto dditchline;
	++in;
	if ((len = gethnum(&in)) == -1)
		goto dditchline;
	if ((n = gethnum(&in)) == -1)
		goto dditchline;
	address = n << 8;
	if ((n = gethnum(&in)) == -1)
		goto dditchline;
	address |= n;
	while (len--) {
		switch (n = gethnum(&in)) {
			case 0:
				ds.addr = address;
				ds.value = n;
				arr_add(&dsym, (void *) &ds);
				++address;
				break;
			case 1:
				/* end record */
				return;
			default:
				goto dditchline;
		}
	}
	if ((n = gethnum(&in)) == -1)
		goto dditchline;
	if (((0x100 - crc) & 0xFF) != n)
		ihwarn("checksum mismatch");
	skipsp(&in);
	if (!skipnl(&in)) {
		ihwarn("exess characters after ihex data");
		goto dditchline;
	}
	++line;
	return;

	dditchline:
	ihwarn("invalid data, skipping rest of line");
	while(!(alfa[(unsigned char) *(in++)] & (CT_NL | CT_NUL)));
	goto dstartline;
}

