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
	AT_BS /* banksel */
};

oc_t ocs14b[] = {
	/* byte operatios */
	{ "clrw",   { 0x01, 0x00 }, { 0xFF, 0x80 }, 2, AT_NA  },
	{ "clrf",   { 0x01, 0x80 }, { 0xFF, 0x80 }, 2, AT_F   },
	{ "movf",   { 0x08, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "decf",   { 0x03, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "incf",   { 0x0A, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "comf",   { 0x09, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "rrf",    { 0x0C, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "rlf",    { 0x0D, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "decfsz", { 0x0B, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "incfsz", { 0x0F, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "movwf",  { 0x00, 0x80 }, { 0xFF, 0x80 }, 2, AT_F   },
	{ "iorwf",  { 0x04, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "andwf",  { 0x05, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "xorwf",  { 0x06, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "subwf",  { 0x02, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "addwf",  { 0x07, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },
	{ "swapf",  { 0x0E, 0x00 }, { 0xFF, 0x00 }, 2, AT_DF  },

	/* bit operations */
	{ "bcf",    { 0x10, 0x00 }, { 0xFC, 0x00 }, 2, AT_BF  },
	{ "bsf",    { 0x14, 0x00 }, { 0xFC, 0x00 }, 2, AT_BF  },
	{ "btfsc",  { 0x18, 0x00 }, { 0xFC, 0x00 }, 2, AT_BF  },
	{ "btfss",  { 0x1C, 0x00 }, { 0xFC, 0x00 }, 2, AT_BF  },

	/* literal operations */
	{ "movlw",  { 0x30, 0x00 }, { 0xFC, 0x00 }, 2, AT_K8  },
	{ "retlw",  { 0x34, 0x00 }, { 0xFC, 0x00 }, 2, AT_K8  },
	{ "iorlw",  { 0x38, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "andlw",  { 0x39, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "xorlw",  { 0x3A, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "sublw",  { 0x3C, 0x00 }, { 0xFE, 0x00 }, 2, AT_K8  },
	{ "addlw",  { 0x3E, 0x00 }, { 0xFE, 0x00 }, 2, AT_K8  },

	/* control operations */
	{ "nop",    { 0x00, 0x00 }, { 0xFF, 0x9F }, 2, AT_NA  },
	{ "return", { 0x00, 0x08 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "retfie", { 0x00, 0x09 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "sleep",  { 0x00, 0x63 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "clrwdt", { 0x00, 0x64 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "call",   { 0x20, 0x00 }, { 0xF8, 0x00 }, 2, AT_K11 },
	{ "goto",   { 0x28, 0x00 }, { 0xF8, 0x00 }, 2, AT_K11 },

	/* deprecated stuff (but still works on PIC i tested) */
	/* tris can only go after clrwdt, or disassembler might be confused */
	{ "option", { 0x00, 0x62 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "tris",   { 0x00, 0x64 }, { 0xFF, 0xFC }, 2, AT_T   },

	/* note to self, do not forget banksel, bankisel, pagesel */

	{ NULL, { 0 }, { 0 }, 0 }, /* important, end of list */
};

void acmp14b(unsigned char *oc, int atype, int argc, int *argv) {
	switch (atype) {
		case AT_DF:
			if (argc != 2)
				flerrexit("wrong number of arguments");
			oc[1] = (argv[1] & 1) << 7 | (argv[0] & 0x7F);
			break;
		case AT_F:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			oc[1] |= argv[0] & 0x7F;
			break;
		case AT_BF:
			if (argc != 2)
				flerrexit("wrong number of arguments");
			oc[1] = (argv[1] & 1) << 7 | (argv[0] & 0x7F);
			oc[0] |= (argv[1] & 6) >> 1;
			break;
		case AT_K8:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			oc[1] |= argv[0] & 0xFF;
			break;
		case AT_K11:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			oc[1] = argv[0] & 0xFF;
			oc[0] |= (argv[0] & 0x700) >> 8;
			break;
		case AT_T:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			oc[1] |= argv[0] & 0x3;
			break;
		case AT_NA:
			if (argc)
				flerrexit("wrong number of arguments");
		case AT_BS:
			break;
	}
}

void adis14b(ioh_t *out, unsigned char *oc, int atype) {
	switch (atype) {
		case AT_DF:
			mfprintf(out, "0x%2x, %i", oc[1] & 0x7F, (oc[1] & 0x80) >> 7);
			break;
		case AT_F:
			mfprintf(out, "0x%2x", oc[1] & 0x7F);
			break;
		case AT_BF:
			mfprintf(out, "0x%2x, %i", oc[1] & 0x7F, ((oc[0] & 3) << 1) | ((oc[1] & 0x80) >> 7));
			break;
		case AT_K8:
				mfprintf(out, "0x%2x", oc[1] & 0xFF);
			break;
		case AT_K11:
			mfprintf(out, "0x%3x", ((oc[0] & 7) << 8) | (oc[1] & 0xFF));
			break;
		case AT_T:
			mfprintf(out, "%i", oc[1] & 3);
			break;
	}
}

int ord14b[2] = { 0, 1 };

arch_t pic14b = {
	ocs14b,
	&acmp14b,
	&adis14b,
	ord14b,
	2,
};
