#include "host.h" /* realloc() */
#include "as.h" /* inss */
#include "misc.h" /* errexit(), flwarn(), infile, line */
#include "arch.h" /* arch */
/* below includes = only for ihex input */
#include "str.h" /* skipsp(), skipnl(), alfa[], hexlookup[], hexnib[] */
#include "dis.h" /* dsym, dsym_t */
#include "io.h"

#define IHLL 16

int pos = 0, lnpos, crc;
unsigned char buf[IHLL];
unsigned int saddr, rtype;

void endln(ioh_t *out) {
	int i;
	crc = 0;
	mfprintf(out, ":%2x%4x%2x", pos, saddr, rtype);
	for (i = 0; i < pos; ++i)
		mfprintf(out, "%2x", buf[i]);
	mfprintf(out, "%2x", (0x100 - crc) & 0xFF);
	mfprint(out, "\n");
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

	address = saddr = rtype = 0;
	line = 1;
	while (ins->type != IT_END) {
		switch (ins->type) {
			case IT_ORG:
				if (pos)
					endln(out);
				address = saddr = ins->d.org.address;
				break;
			case IT_DAT:
				addb(out, ins->d.data.value & 0xFF);
				++address;
				addb(out, (ins->d.data.value & 0xFF00) >> 8);
				++address;
				break;
			case IT_INS:
				for (i = 0; i < ins->d.ins.len; ++i) {
					addb(out, ins->d.ins.oc[arch->insord[i]]);
					++address;
				}
				break;
		}
		++ins;
	}
	if (pos)
		endln(out);
	mfprint(out, ":00000001FF\n");
}

void ihwarn(char *msg) {
	flwarn(infile, line, msg);
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

	arr_new(&dsym, sizeof(dsym_t));
	dstartline:
	crc = 0;
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
	crc += len;
	if ((n = gethnum(&in)) == -1)
		goto dditchline;
	crc += n;
	addr = n << 8;
	if ((n = gethnum(&in)) == -1)
		goto dditchline;
	crc += n;
	addr |= n;
	rtype = gethnum(&in);
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
			goto dditchline;
	}
	if ((n = gethnum(&in)) == -1)
		goto dditchline;
	if (((0x100 - crc) & 0xFF) != n)
		ihwarn("checksum mismatch");
	if (rtype == 1)
		return;
	skipsp(&in);
	if (!skipnl(&in)) {
		ihwarn("exess characters after ihex data");
		goto dditchline;
	}
	++line;
	goto dstartline;

	dditchline:
	ihwarn("invalid data, skipping rest of line");
	while(!(alfa[(unsigned char) *(in++)] & (CT_NL | CT_NUL)));
	goto dstartline;
}
