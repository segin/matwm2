#include "host.h" /* NULL, strlen() */
#include "dis.h" /* dsym_t */
#include "misc.h" /* fawarn(), errexit(), infile, address */
#include "arch.h"
#include "str.h" /* hexnib[] */
#include "mem.h" /* string_t stuff */

arr_t dsym = { NULL, 0, 0, 0 };
int lbpos;

void daddstr(char *s) {
	while ((linebuf[lbpos] = *(s++))) {
		++lbpos;
		if (lbpos > sizeof(linebuf))
			errexit("line buffer overflow");
	}
}

void _daddhex(int n, int l) {
	while (l--) {
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
	int i, j, c = dsym.count;
	oc_t *oc;
	unsigned char inop[6];
	string_t rb;

	vstr_new(&rb);
	while (c) {
		oc = arch->ocs;
		lbpos = 0;
		address = sym->addr;
		_daddhex(address, 8);
		daddstr(" (");
		while (oc->name != NULL) {
			if (oc->len > c) /* this is to prevent disaster */
				goto docf;
			for (i = 0; ((sym + arch->insord[i])->value & oc->imask[i]) == oc->oc[i] && i < oc->len; ++i);
			if (i == oc->len) {
				for (j = 0; j < oc->len; ++j)
					inop[j] = (sym + arch->insord[j])->value;
				for (i = 0; i < oc->len; ++i)
					_daddhex(inop[i], 2);
				daddstr("): ");
				daddstr(oc->name);
				daddstr(" ");
				arch->adis(inop, oc->atype);
				sym += oc->len;
				c -= oc->len;
				break;
			}
			docf:
			++oc;
		}
		if (i != oc->len) {
			for (j = 0; j < arch->align; ++j)
				inop[j] = (sym + arch->insord[j])->value;
			daddstr("): ");
			fawarn(infile, line, "invalid opcode");
			daddstr("[invalid opcode]");
			++address;
			sym += arch->align;
			c -= arch->align;
		}
		daddstr("\n");
		vstr_addl(&rb, linebuf, lbpos);
	}
	*ret = rb.data;
	return rb.len;
}
