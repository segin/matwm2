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
	{ "addwf", 0x0700, 0x0000, AT_DF },
	/* note to self, do not forget banksel, bankisel, pagesel */
	{ NULL, 0, 0, 0 }, /* important, end of list */
};

int acmp14b(int oc, int atype, int argc, int *argv) {
	switch (atype) {
		case AT_DF:
			if (argc != 2)
				/* errexit("wrong number of arguments") */;
			oc |= (argv[0] & 0xFF) | ((argv[1] & 1) << 7);
			break;
	}
	return oc;
}

arch_t pic14b = {
	.ocs = ocs14b,
	.acmp = &acmp14b,
};

