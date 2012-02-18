#include "host.h" /* realloc() */
#include "mem.h" /* BLOCK */
#include "as.h" /* inss */
#include "misc.h" /* errexit(), flwarn() */
#include "arch.h" /* arch */
/* below includes = only for ihex input */
#include "str.h" /* skipsp(), skipnl(), alfa[], hexlookup[], hexnib[] */
#include "main.h" /* infile, line */
#include "dis.h" /* dsym, dsym_t */

int dosnl = 0;
char *out;
int pos = 0, crc;
int mem, len;
unsigned int addr, saddr, rtype;
unsigned char buf[16];

void sethb(unsigned char b) {
	*(out + (len++)) = hexnib[(b & 0xF0) >> 4];
	*(out + (len++)) = hexnib[b & 0xF];
	crc += b;
}

void endln(void) {
	int i, res;

	/* make sure enough memory is ready */
	res = len + (((pos + 5) * 2) + 3);
	if (res < len)
		errexit(":-o integer overflow");
	while (res > mem) {
		if (mem + BLOCK < mem)
			errexit("integer overflow :(");
		mem += BLOCK;
		out = (char *) realloc((void *) out, mem);
		if (out == NULL)
			errexit("out of memory");
	}
	/* write line */
	out[len++] = ':';
	crc = 0;
	sethb(pos);
	sethb((saddr & 0xFF00) >> 8);
	sethb(saddr & 0xFF);
	sethb(rtype);
	for (i = 0; i < pos; ++i)
		sethb(buf[i]);
	sethb((0x100 - crc) & 0xFF);
	if (dosnl)
		out[len++] = '\r';
	out[len++] = '\n';
	pos = 0;
	saddr = addr;
}

void addb(unsigned char b) {
	buf[pos++] = b;
	crc += b;
	if (pos == 16)
		endln();
}

int getihex(char **ret) {
	int i;
	ins_t *ins = (ins_t *) inss.data;

	addr = saddr = mem = len = rtype = 0;
	out = NULL;
	while (ins->type != IT_END) {
		switch (ins->type) {
			case IT_ORG:
				if (pos)
					endln();
				addr = saddr = ins->d.org.address;
				break;
			case IT_DAT:
				addb(ins->d.data.value & 0xFF);
				++addr;
				addb((ins->d.data.value & 0xFF00) >> 8);
				++addr;
				break;
			case IT_INS:
				for (i = 0; i < ins->d.ins.len; ++i) {
					addb(ins->d.ins.oc[arch->insord[i]]);
					++addr;
				}
				break;
		}
		++ins;
	}
	if (pos)
		endln();

	/* spit out end record */
	rtype = 1;
	saddr = 0;
	endln();

	out[len] = 0;
	*ret = out;
	return len;
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

