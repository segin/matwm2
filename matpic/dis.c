#include "arch.h"
#include "str.h" /* skipsp(), skipnl(), alfa[], hexlookup[] */
#include "mem.h"
#include "misc.h" /* flerrexit() */
#include "as.h" /* file, address */

/* we store stuff in this way cause we intend later also support coff */
typedef struct {
	unsigned char value;
	unsigned int addr;
} dsym_t;

arr_t symbols;

void dwarn(char *msg) {
	fawarn(file, address, msg);
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
	int n, len;
	dsym_t ds;

	arr_new(&symbols, sizeof(dsym_t));
	dstartline:
	do {
		skipsp(&in);
	} while (skipnl(&in));
	if (*in == 0) {
		dwarn("no end record");
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
				arr_add(&symbols, (void *) &ds);
				++address;
				break;
			case 1:
				/* end record */
				return;
			default:
				goto dditchline;
		}
	}
	return;

	dditchline:
	dwarn("invalid data, skipping rest of line");
	while(!(alfa[(unsigned char) *(in++)] & (CT_NL | CT_NUL)));
	goto dstartline;
}

