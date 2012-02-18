#include "host.h" /* NULL */
#include "dis.h" /* dsym_t */
#include "misc.h" /* fawarn(), errexit() */
#include "main.h" /* infile, address */
#include "arch.h"
#include "str.h" /* hexnib[] */

arr_t dsym = { NULL, 0, 0, 0 };
char linebuf[512];
int lbpos;

void dwarn(char *msg) {
	fawarn(infile, address, msg);
}

void daddstr(char *s) {
	while (linebuf[lbpos++] = *(s++))
		if (lbpos > sizeof(linebuf))
			errexit("line buffer overflow");
}

void _daddhex(int n, int l) {
	while (--l) {
		linebuf[lbpos++] = hexnib[(n >> (l << 2)) & 0x0F];
		if (lbpos > sizeof(linebuf))
			errexit("line buffer overflow");
	}
}

void daddhex(int n, int l) {
	daddstr("0x");
	_daddhex(n, l);
}

int disassemble(char **ret) {
	dsym_t *sym = (dsym_t *) dsym.data;
	int i, c = dsym.count;
	oc_t *oc;
	unsigned char inop[6];

	while (c) {
		oc = arch->ocs;
		lbpos = 0;
		while (oc->name != NULL) {
			if (oc->len > c) /* this is to prevent disaster */
				goto docf;
			for (i = 0; ((sym + arch->insord[i])->value & oc->imask[i]) == oc->oc[i] && i < oc->len; ++i);
			if (i == oc->len) {
				daddstr(oc->name);
				daddstr(" ");
				sym += oc->len;
				c -= oc->len;
				break;
			}
			docf:
			++oc;
		}
		if (i != oc->len) {
			daddstr("[invalid opcode]");
			sym += arch->align;
			c -= arch->align;
		}
		printf("%s\n", linebuf);
	}
}

