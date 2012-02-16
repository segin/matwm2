#include "host.h" /* realloc() */
#include "mem.h" /* BLOCK */
#include "as.h" /* inss, address */
#include "misc.h" /* errexit() */
#include "arch.h" /* arch */

char *out;
int pos = 0, crc;
int mem, len;
int addr, saddr, rtype = 0;
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
	int i, res = mem + (((pos + 5) * 2) + 2);

	/* make sure enough memory is ready */
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
	sethb((crc & 0xFF) ^ 0xFF);
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

	addr = saddr = mem = len = 0;
	out = NULL;
	while (ins->type != IT_END) {
		switch (ins->type) {
			case IT_ORG:
				endln();
				addr = saddr = ins->org.address;
				break;
			case IT_DAT:
				addb((ins->data.value & 0xFF00) >> 8);
				++addr;
				addb(ins->data.value & 0xFF);
				++addr;
				break;
			case IT_INS:
				for (i = 0; i < ins->ins.len; ++i) {
					addb(ins->ins.oc[i]);
					++addr;
				}
				break;
		}
		++ins;
	}
	endln();
	out[len] = 0;
	*ret = out;
	return len;
}

