#include "mem.h" /* BLOCK */
#include "as.h" /* inss */
#include "misc.h" /* errexit(), flwarn(), infile, line */
#include "arch.h" /* arch */
/* below includes = only for ihex input */
#include "str.h" /* skipsp(), skipnl(), ctype, hexlookup[], hexnib[] */
#include "dis.h" /* dsym, dsym_t */
#include "io.h"

#define IHLL 16

unsigned int saddr;
unsigned char buf[IHLL];
int pos = 0, crc;

void endln(ioh_t *out) {
	int i;
	mfprintf(out, ":%2x%4x00", pos, saddr);
	for (i = 0; i < pos; ++i)
		mfprintf(out, "%2x", buf[i]);
	crc += saddr >> 8;
	crc += saddr & 0xFF;
	mfprintf(out, "%2x", (0x100 - (crc + pos)) & 0xFF);
	mfprint(out, "\n");
	crc = 0;
	pos = 0;
	saddr = address;
}

void addb(ioh_t *out, unsigned char b) {
	buf[pos++] = b;
	crc += b;
	if (pos == 16)
		endln(out);
}

void getihex(ioh_t *out) {
	int i;
	ins_t *ins = (ins_t *) inss.data;

	address = saddr = 0;
	crc = 0;
	line = 1;
	while (ins->type != IT_END) {
		switch (ins->type) {
			case IT_ORG:
				if (pos)
					endln(out);
				address = saddr = ins->d.org.address;
				break;
			case IT_DAT:
				for (i = 0; i < arch->dlen; ++i) {
					addb(out, (ins->d.data.value & (0xFF << (8 * arch->dord[i]))) >> (8 * arch->dord[i]));
					address += arch->dlen;
				}
				break;
			case IT_INS:
				address += ins->d.ins.len;
				for (i = 0; i < ins->d.ins.len; ++i)
					addb(out, ins->d.ins.oc[arch->insord[i]]);
				break;
		}
		++ins;
	}
	if (pos)
		endln(out);
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

void readihex(char *in) {
	int n, len, crc, rtype, addr;
	dsym_t ds;

	file = infile;
	arr_new(&dsym, sizeof(dsym_t));
	dstartline:
	crc = 0;
	--line;
	do {
		skipsp(&in);
		++line;
	} while (skipnl(&in));
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
	switch (rtype) {
		case 0:
			while (len--) {
				n = gethnum(&in);
				crc += n;
				ds.addr = addr;
				ds.value = n;
				arr_add(&dsym, (void *) &ds);
				++addr;
			}
			break;
		case 1:
			/* end record */
			while (len--) {
				n = gethnum(&in);
				crc += n;
			}
			break;
		case -1:
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
	++line;
	goto dstartline;

	dinval:
	flwarn("invalid data, skipping rest of line");
	dnextline:
	while(!(ctype(*(in++)) & (CT_NL | CT_NUL)));
	skipnl(&in);
	++line;
	goto dstartline;
}
