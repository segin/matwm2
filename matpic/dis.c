#include <stdlib.h> /* NULL */
#include "dis.h" /* dsym_t */
#include "misc.h" /* fawarn(), errexit(), infile, address */
#include "arch.h"
#include "str.h" /* hexnib[] */
#include "io.h"

arr_t dsym = { NULL, 0, 0, 0 }; /* these need to be 0 so cleanup() before disassemble won't fail */

void disassemble(ioh_t *out) {
	dsym_t *sym = (dsym_t *) dsym.data;
	int i = 0, c = dsym.count;
	oc_t *oc;
	unsigned char inop[6];
	unsigned char *end, *bufp = (unsigned char *) inbuf.data;

	while (c > 0) {
		address = sym->addr;
		end = bufp + sym->len;
		while (bufp != end) {
			oc = arch->ocs;
			mfprintf(out, "%8x (", address / arch->align);
			while (oc->name != NULL) {
				if (oc->len > end - bufp) /* this is to prevent disaster */
					goto docf;
				for (i = 0; (bufp[arch->ord[i % arch->align]] & oc->imask[i]) == oc->oc[i] && i < oc->len; ++i);
				if (i == oc->len) {
					for (i = 0; i < oc->len; ++i)
						inop[i] = bufp[arch->ord[i % arch->align]];
					for (i = 0; i < oc->len; ++i)
						mfprintf(out, "%2x", inop[i]);
					mfprintf(out, "): %s ", oc->name);
					arch->adis(out, inop, oc->atype);
					bufp += oc->len;
					address += oc->len;
					break;
				}
				docf:
				++oc;
			}
			if (oc->name == NULL) {
				if (arch->align > end - bufp) {
					for (i = 0; i < end - bufp; ++i)
						mfprintf(out, "%2x", bufp[i]);
					mfprint(out, "): data not divisible by alignment\n");
					break;
				} else {
					for (i = 0; i < arch->align; ++i)
						mfprintf(out, "%2x", bufp[arch->ord[i]]);
					mfprint(out, "): data 0x");
					for (i = 0; i < arch->align; ++i)
						mfprintf(out, "%2x", bufp[arch->ord[i]]);
					bufp += arch->align;
					address += oc->len;
				}
			}
			mfprint(out, "\n");
		}
		--c;
		++sym;
	}
}
