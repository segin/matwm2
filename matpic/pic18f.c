/*************************************************
 * architecture definitions for 18F... PIC chips *
 *************************************************/

#include "arch.h"
#include "pic18f.h"

enum atype {
	AT_NA,
	AT_DF,
	AT_FF,
	AT_F,
	AT_BF,
	AT_K4,
	AT_K8,
	AT_K20,
	AT_KS,
	AT_FK,
	AT_S,
	AT_FSR,
	AT_ULNK,
	AT_SS,
	AT_SF,
	AT_R8,
	AT_R11
};

oc_t ocs18f[] = {
	/* byte operations */
	{ "addwf",  { 0x24, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "addwfc", { 0x20, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "andwf",  { 0x14, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "clrf",   { 0x6A, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "comf",   { 0x1C, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "cpfseq", { 0x62, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "cpfsgt", { 0x64, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "cpfslt", { 0x60, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "decf",   { 0x04, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "decfsz", { 0x2C, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "dcfsnz", { 0x4C, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "incf",   { 0x28, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "incfsz", { 0x3C, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "infsnz", { 0x48, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "iorwf",  { 0x10, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "movf",   { 0x50, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "movwf",  { 0x6E, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "mulwf",  { 0x02, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "negf",   { 0x6C, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "rlcf",   { 0x34, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "rlncf",  { 0x44, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "rrcf",   { 0x30, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "rrncf",  { 0x40, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "setf",   { 0x68, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "subfwb", { 0x54, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "subwf",  { 0x5C, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "subwfb", { 0x58, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "swapf",  { 0x38, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "tstfsz", { 0x66, 0x00 }, { 0xFE, 0x00 }, 2, AT_F   },
	{ "xorwf",  { 0x18, 0x00 }, { 0xFC, 0x00 }, 2, AT_DF  },
	{ "movff",  { 0xC0, 0x00, 0xF0, 0x00 }, /* yes double size */
	            { 0xF0, 0x00, 0xF0, 0x00 }, 4, AT_FF  },

	/* bit operations */
	{ "bcf",    { 0x90, 0x00 }, { 0xF0, 0x00 }, 2, AT_BF  },
	{ "bsf",    { 0x80, 0x00 }, { 0xF0, 0x00 }, 2, AT_BF  },
	{ "btfsc",  { 0xB0, 0x00 }, { 0xF0, 0x00 }, 2, AT_BF  },
	{ "btfss",  { 0xA0, 0x00 }, { 0xF0, 0x00 }, 2, AT_BF  },
	{ "btg",    { 0x70, 0x00 }, { 0xF0, 0x00 }, 2, AT_BF  },

	/* literal operations */
	{ "addlw",  { 0x0F, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "andlw",  { 0x0B, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "iorlw",  { 0x09, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "movlb",  { 0x01, 0x00 }, { 0xFF, 0xF0 }, 2, AT_K4  },
	{ "movlw",  { 0x0E, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "mullw",  { 0x0D, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "retlw",  { 0x0C, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "sublw",  { 0x08, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "xorlw",  { 0x0A, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "lfsr",   { 0xEE, 0x00, 0xF0, 0x00 },
	            { 0xFF, 0xC0, 0xFF, 0x00 }, 4, AT_FK },

	/* control operations */
	{ "bc",     { 0xE2, 0x00 }, { 0xFF, 0x00 }, 2, AT_R8  },
	{ "bn",     { 0xE6, 0x00 }, { 0xFF, 0x00 }, 2, AT_R8  },
	{ "bnc",    { 0xE3, 0x00 }, { 0xFF, 0x00 }, 2, AT_R8  },
	{ "bnn",    { 0xE7, 0x00 }, { 0xFF, 0x00 }, 2, AT_R8  },
	{ "bnov",   { 0xE5, 0x00 }, { 0xFF, 0x00 }, 2, AT_R8  },
	{ "bnz",    { 0xE1, 0x00 }, { 0xFF, 0x00 }, 2, AT_R8  },
	{ "bov",    { 0xE4, 0x00 }, { 0xFF, 0x00 }, 2, AT_R8  },
	{ "bra",    { 0xD0, 0x00 }, { 0xF8, 0x00 }, 2, AT_R11 },
	{ "bz",     { 0xE0, 0x00 }, { 0xFF, 0x00 }, 2, AT_R8  },
	{ "clrwdt", { 0x00, 0x04 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "daw",    { 0x00, 0x07 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "nop",    { 0x00, 0x00 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "nop",    { 0xF0, 0x00 }, { 0xF0, 0x00 }, 2, AT_NA  },
	{ "pop",    { 0x00, 0x06 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "push",   { 0x00, 0x05 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "rcall",  { 0xD8, 0x00 }, { 0xF8, 0x00 }, 2, AT_R11 },
	{ "reset",  { 0x00, 0xFF }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "retfie", { 0x00, 0x10 }, { 0xFF, 0xFE }, 2, AT_S   },
	{ "retlw",  { 0x0C, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8  },
	{ "return", { 0x00, 0x12 }, { 0xFF, 0xFE }, 2, AT_S   },
	{ "sleep",  { 0x00, 0x03 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "call",   { 0xEC, 0x00, 0xF0, 0x00 },
	            { 0xFE, 0x00, 0xF0, 0x00 }, 4, AT_KS },
	{ "goto",   { 0xEF, 0x00, 0xF0, 0x00 },
	            { 0xFF, 0x00, 0xF0, 0x00 }, 4, AT_K20 },

	/* table operations - in name of sanity i modified the mnemonics (or drop the " name of ") */
	{ "tblrd",  { 0x00, 0x08 }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "tblrdi", { 0x00, 0x09 }, { 0xFF, 0xFF }, 2, AT_NA  }, /* post-increment */
	{ "tblrdd", { 0x00, 0x0A }, { 0xFF, 0xFF }, 2, AT_NA  }, /* post-decrement */
	{ "tblrdp", { 0x00, 0x0B }, { 0xFF, 0xFF }, 2, AT_NA  }, /* pre-increment */
	{ "tblwt",  { 0x00, 0x0C }, { 0xFF, 0xFF }, 2, AT_NA  },
	{ "tblwti", { 0x00, 0x0D }, { 0xFF, 0xFF }, 2, AT_NA  }, /* post-increment */
	{ "tblwtd", { 0x00, 0x0E }, { 0xFF, 0xFF }, 2, AT_NA  }, /* post-decrement */
	{ "tblwtp", { 0x00, 0x0F }, { 0xFF, 0xFF }, 2, AT_NA  }, /* pre-increment */

	/* "extended instruction set" */
	{ "addulnk", { 0xE8, 0xC0 }, { 0xFF, 0xC0 }, 2, AT_ULNK }, /* must go before addfsr */
	{ "addfsr",  { 0xE8, 0x00 }, { 0xFF, 0x00 }, 2, AT_FSR  },
	{ "callw",   { 0x00, 0x14 }, { 0xFF, 0xFF }, 2, AT_NA   },
	{ "pushl",   { 0xEA, 0x00 }, { 0xFF, 0x00 }, 2, AT_K8   },
	{ "subulnk", { 0xE9, 0xC0 }, { 0xFF, 0xC0 }, 2, AT_ULNK }, /* must go before addfsr */
	{ "subfsr",  { 0xE9, 0x00 }, { 0xFF, 0x00 }, 2, AT_FSR  },
	{ "movsf",   { 0xEB, 0x00, 0xF0, 0x00 },
	             { 0xFF, 0x80, 0xF0, 0x00 }, 4, AT_SF },
	{ "movss",   { 0xEB, 0x80, 0xF0, 0x00 },
	             { 0xFF, 0x80, 0xF0, 0x00 }, 4, AT_SS },

	/* note to self, do not forget pagesel */

	{ NULL, { 0 }, { 0 }, 0 }, /* important, end of list */
};

void acmp18f(unsigned char *oc, int atype, int argc, signed long long *argv) {
	signed long n;
	switch (atype) {
		case AT_DF:
			if (argc < 1 || argc > 3)
				flerrexit("wrong number of arguments");
			if (argc < 3)
				argv[2] = 0;
			if (argc < 2)
				argv[1] = 1;
			if (argv[1] > 1 || argv[1] < 0 || argv[2] > 1 || argv[2] < 0)
				flwarn("argument out of range, truncated");
			oc[1] = ntt(argv[0]);
			oc[0] |= ((argv[1] & 1) << 1) | (argv[2] & 1);
			break;
		case AT_F:
			if (argc < 1 || argc > 2)
				flerrexit("wrong number of arguments");
			if (argc < 2)
				argv[1] = 0;
			if (argv[1] > 1 || argv[1] < 0)
				flwarn("argument out of range, truncated");
			oc[1] = ntt(argv[0]);
			oc[0] |= argv[1] & 1;
			break;
		case AT_FF:
			if (argc != 2)
				flerrexit("wrong number of arguments");
			oc[1] = ntt(argv[0]);
			oc[0] |= (ntt(argv[0]) >> 8) & 0x0F;
			oc[3] = ntt(argv[1]);
			oc[2] |= (ntt(argv[1]) >> 8) & 0x0F;
			break;
		case AT_BF:
			if (argc < 2 || argc > 3)
				flerrexit("wrong number of arguments");
			if (argc < 3)
				argv[2] = 0;
			if (argv[1] > 7 || argv[1] < 0 || argv[2] > 1 || argv[2] < 0)
				flwarn("argument out of range, truncated");
			oc[1] = ntt(argv[0]);
			oc[0] |= ((argv[1] & 7) << 1) | (argv[2] & 1);
			break;
		case AT_K4:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			oc[1] |= ntt(argv[0]) & 0x0F;
			break;
		case AT_K8:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			oc[1] |= ntt(argv[0]);
			break;
		case AT_R8:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			n = ntt(argv[0]) - (address + 1);
			if (n > 127 || n < -128)
				flwarn("relative adress out of bounds, truncated");
			oc[1] |= n & 0xFF;
			break;
		case AT_K20:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			oc[1] = ntt(argv[0]);
			oc[3] |= (ntt(argv[0]) >> 8);
			oc[2] |= (ntt(argv[0]) >> 16) & 0x0F;
			break;
		case AT_KS:
			if (argc < 1 || argc > 2)
				flerrexit("wrong number of arguments");
			if (argc < 2)
				argv[1] = 0;
			if (argv[1] > 1 || argv[1] < 0)
				flwarn("argument out of range, truncated");
			oc[1] = ntt(argv[0]);
			oc[0] |= argv[1] & 1;
			oc[3] = ntt(argv[0]) >> 8;
			oc[2] |= (ntt(argv[0]) >> 16) & 0x0F;
			break;
		case AT_FK:
			if (argc != 2)
				flerrexit("wrong number of arguments");
			if (argv[0] > 3 || argv[0] < 0)
				flwarn("argument out of range, truncated");
			oc[1] |= ((ntt(argv[1]) >> 8) & 0x0F) | ((argv[0] & 3) << 4);
			oc[3] = ntt(argv[1]);
			break;
		case AT_R11:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			n = ntt(argv[0]) - (address + 1);
			if (n > 1023 || n < -1024)
				flwarn("relative adress out of bounds, truncated");
			oc[1] = n;
			oc[0] |= (n >> 8) & 7;
			break;
		case AT_S:
			if (argc > 1)
				flerrexit("too many arguments");
			if (argc < 1)
				argv[0] = 0;
			if (argv[0] > 1 || argv[0] < 0)
				flwarn("argument out of range, truncated");
			oc[1] |= argv[0] & 1;
			break;
		case AT_FSR:
			if (argc != 2)
				flerrexit("wrong number of arguments");
			if (argv[0] > 3 || argv[0] < 0)
				flwarn("argument out of range, truncated");
			oc[1] = (ntt(argv[1]) & 0x3F) | ((argv[0] & 3) << 6);
			break;
		case AT_ULNK:
			if (argc != 1)
				flerrexit("wrong number of arguments");
			oc[1] |= ntt(argv[0]) & 0x3F;
			break;
		case AT_SF:
			if (argc != 2)
				flerrexit("wrong number of arguments");
			oc[1] = ntt(argv[0]) & 0x7F;
			oc[3] = ntt(argv[1]);
			oc[2] |= (ntt(argv[1]) >> 8) & 0x0F;
			break;
		case AT_SS:
			if (argc != 2)
				flerrexit("wrong number of arguments");
			oc[1] = ntt(argv[0]) & 0x7F;
			oc[3] = ntt(argv[1]) & 0x7F;
			break;
		case AT_NA:
			if (argc)
				flerrexit("wrong number of arguments");
	}
}

void adis18f(ioh_t *out, unsigned char *oc, int atype) {
	switch (atype) {
		case AT_DF:
			mfprintf(out, "0x%2x, %i, %i", oc[1], (oc[0] & 2) >> 1, oc[0] & 1);
			break;
		case AT_F:
			mfprintf(out, "0x%2x, %i", oc[1], oc[0] & 1);
			break;
		case AT_FF:
			mfprintf(out, "0x%3x, 0x%3x", oc[1] | ((oc[0] & 0x0F) << 8), oc[3] | ((oc[2] & 0x0F) << 8));
			break;
		case AT_BF:
			mfprintf(out, "0x%2x, %i, %i", oc[1], (oc[0] & 0x0E) >> 1, oc[0] & 1);
			break;
		case AT_K4:
			mfprintf(out, "0x%x", oc[1] & 0x0F);
			break;
		case AT_K8:
			mfprintf(out, "0x%2x", oc[1]);
			break;
		case AT_R8:
			mfprintf(out, "%d", ttn(oc[1], 8));
			break;
		case AT_K20:
			mfprintf(out, "0x%5x", oc[1] | (oc[3] << 8) | ((oc[2] & 0x0F) << 16));
			break;
		case AT_FK:
			mfprintf(out, "%i, 0x%3x", (oc[1] >> 4) & 3, ((oc[1] & 0x0F) << 8) | oc[3]);
			break;
		case AT_R11:
			mfprintf(out, "%d", ttn(oc[1] | ((oc[0] & 7) << 8), 11));
			break;
		case AT_KS:
			mfprintf(out, "0x%5x, %i", oc[1] | (oc[3] << 8) | ((oc[2] & 0x0F) << 16), oc[0] & 1);
			break;
		case AT_S:
			mfprintf(out, "%i", oc[1] & 1);
			break;
		case AT_FSR:
			mfprintf(out, "%i, 0x%2x", oc[1] >> 6, oc[1] & 0x3F);
			break;
		case AT_ULNK:
			mfprintf(out, "0x%2x", oc[1] & 0x3F);
			break;
		case AT_SF:
			mfprintf(out, "0x%2x, 0x%3x", oc[1] & 0x7F, oc[3] | ((oc[2] & 0x0F) << 8));
			break;
		case AT_SS:
			mfprintf(out, "0x%2x, 0x%2x", oc[1] & 0x7F, oc[3] & 0x7F);
			break;
	}
}

unsigned char mask18f[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

int ord18f[4] = { 1, 0, 3, 2 };

arch_t pic18f = {
	"pic18f",
	ocs18f,
	&acmp18f,
	&adis18f,
	mask18f,
	ord18f,
	2,
	4
};
