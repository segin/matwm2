/*********************************************************************
 * architecture definitions for PIC chips with 14bit instruction set *
 *********************************************************************/

#include "pic14b.h"

enum atype {
	AT_NA,
	AT_DF,
	AT_F,
	AT_BF,
	AT_K8,
	AT_K11,
	AT_T, /* for tris instruction */
	AT_BS, /* banksel */
};

oc_t ocs14b[] = {
	{ "addwf", { 0x07, 0x00 }, { 0x00, 0x00 }, 2, AT_DF },
	/* note to self, do not forget banksel, bankisel, pagesel */
	{ NULL, 0, 0, 0 }, /* important, end of list */
};

void acmp14b(unsigned char *oc, int atype, int argc, int *argv) {
	switch (atype) {
		case AT_DF:
			if (argc != 2)
				aerrexit("wrong number of arguments");
			oc[1] = (argv[1] & 1) << 7 | argv[0];
			break;
	}
}

arch_t pic14b = {
	.ocs = ocs14b,
	.acmp = &acmp14b,
	.align = 2,
};

