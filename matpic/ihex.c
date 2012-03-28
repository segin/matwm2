#include "mem.h"
#include "as.h" /* inss */
#include "misc.h" /* errexit(), flwarn(), file, infile */
#include "arch.h" /* arch */
/* below includes = only for ihex input */
#include "str.h" /* skipsp(), skipnl(), ctype, hexlookup[] */
#include "dis.h" /* dsym, dsym_t */
#include "io.h"
#include "lineno.h"

#define IHLL 16

unsigned char buf[IHLL];
int pos = 0, crc;

void endln(ioh_t *out) {
	int i;
	mfprintf(out, ":%2x%4x00", pos, address);
	for (i = 0; i < pos; ++i)
		mfprintf(out, "%2x", buf[i]);
	crc += address >> 8;
	crc += address & 0xFF;
	mfprintf(out, "%2x", (0x100 - (crc + pos)) & 0xFF);
	mfprint(out, "\n");
	address += pos;
	crc = 0;
	pos = 0;
}

void ihex_write(ioh_t *out) {
	ins_t *ins = (ins_t *) inss.data;
	char *bufp = outbuf.data;

	address = 0;
	crc = 0;
	while (ins->head.type != IT_END) {
		if (ins->head.type == IT_ORG) {
				address = ins->org.address * arch->align;
				while (bufp != ins->org.end) {
					buf[pos++] = *bufp;
					crc += *bufp;
					if (pos == 16)
						endln(out);
					++bufp;
				}
				if (pos)
					endln(out);
		}
		++ins;
	}
	mfprint(out, ":00000001FF\n");
}

int gethnum(char **src) {
	int r, l = hexlookup[(unsigned char) **src];
	if (l == 16) /* really check inbetween, in case we hit terminating 0 */
		return -1;
	r = hexlookup[(unsigned char) *((*src) + 1)];
	if (r == 16)
		return -1;
	*src += 2;
	return (l << 4) | r;
}

void ihex_read(char *in) {
	int len, crc, rtype, n;
	unsigned long addr;
	char c;
	dsym_t ds;

	arr_new(&dsym, sizeof(dsym_t));
	vstr_new(&inbuf);
	lineno_init();
	lineno_pushfile(infile, 1, 0);
	dstartline:
	crc = 0;
	skipsp(&in);
	while (skipnl(&in)) {
		skipsp(&in);
		lineno_inc();
	}
	if (*in == 0) {
		flwarn("no end record");
		return;
	}
	if (*in != ':')
		goto dinval;
	++in;
	if ((len = gethnum(&in)) == -1)
		goto dinval;
	crc += len;
	if ((n = gethnum(&in)) == -1)
		goto dinval;
	crc += n;
	addr = n << 8;
	if ((n = gethnum(&in)) == -1)
		goto dinval;
	crc += n;
	addr |= n;
	if ((rtype = gethnum(&in)) == -1)
		goto dinval;
	crc += rtype;
	ds.addr = addr;
	switch (rtype) {
		case 0:
			ds.len = len;
			arr_add(&dsym, (void *) &ds);
			while (len--) {
				if ((n = gethnum(&in)) == -1)
					goto dinval;
				crc += n;
				c = n;
				vstr_addl(&inbuf, (char *) &c, 1);
				++addr;
			}
			break;
		case 1:
			/* end record */
			while (len--) {
				if ((n = gethnum(&in)) == -1)
					goto dinval;
				crc += n;
			}
			break;
		default:
			goto dinval;
	}
	if ((n = gethnum(&in)) == -1)
		goto dinval;
	if (((0x100 - crc) & 0xFF) != n)
		flwarn("checksum mismatch");
	if (rtype == 1)
		return;
	skipsp(&in);
	if (!skipnl(&in)) {
		flwarn("exess characters after ihex data");
		goto dnextline;
	}
	lineno_inc();
	goto dstartline;

	dinval:
	flwarn("invalid data, skipping rest of line");
	dnextline:
	while(!(ctype(*(in++)) & (CT_NL | CT_NUL)));
	skipnl(&in);
	lineno_inc();
	goto dstartline;
}
