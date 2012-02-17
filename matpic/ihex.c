#include "host.h" /* realloc() */
#include "mem.h" /* BLOCK */
#include "as.h" /* inss, address */
#include "misc.h" /* errexit() */
#include "arch.h" /* arch */

int dosnl = 0;
char *out;
int pos = 0, crc;
int mem, len;
unsigned int addr, saddr, rtype;
unsigned char buf[16];

char hexnib[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F',
};

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
				for (i = ins->d.ins.len - 1; i >= 0; --i) {
					addb(ins->d.ins.oc[i]);
					++addr;
				}
				break;
		}
		++ins;
	}
	endln();

	/* spit out end record */
	rtype = 1;
	saddr = 0;
	endln();

	out[len] = 0;
	*ret = out;
	return len;
}

