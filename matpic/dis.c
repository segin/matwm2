#include "host.h" /* NULL, strlen() */
#include "dis.h" /* dsym_t */
#include "misc.h" /* fawarn(), errexit(), infile, address */
#include "arch.h"
#include "str.h" /* hexnib[] */
#include "io.h"

arr_t dsym = { NULL, 0, 0, 0 };

void disassemble(ioh_t *out) {
	dsym_t *sym = (dsym_t *) dsym.data;
	int i, c = dsym.count;
	oc_t *oc;
	unsigned char inop[6];

	while (c) {
		oc = arch->ocs;
		address = sym->addr;
		mfprintf(out, "%8x (", address >> arch->align);
		while (oc->name != NULL) {
			if (oc->len > c) /* this is to prevent disaster */
				goto docf;
			for (i = 0; ((sym + arch->insord[i])->value & oc->imask[i]) == oc->oc[i] && i < oc->len; ++i);
			if (i == oc->len) {
				for (i = 0; i < oc->len; ++i)
					inop[i] = (sym + arch->insord[i])->value;
				for (i = 0; i < oc->len; ++i)
					mfprintf(out, "%2x", inop[i]);
				mfprintf(out, "): %s ", oc->name);
				arch->adis(out, inop, oc->atype);
				mfprint(out, "\n");
				sym += oc->len;
				c -= oc->len;
				break;
			}
			docf:
			++oc;
		}
		if (i != oc->len) {
			for (i = 0; i < arch->align; ++i)
				inop[i] = (sym + arch->insord[i])->value;
			mfprint(out, "): [invalid opcode]\n");
			fawarn(infile, line, "invalid opcode");
			++address;
			sym += arch->align;
			c -= arch->align;
		}
	}
}
