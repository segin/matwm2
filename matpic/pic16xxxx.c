/* pic16xxxx disassembler
 *
 * Copyright (c) 2011 Mattis Michel
 *
 * TODO:
 *  - complete instruction set (option, tris, etc)
 *  - verification of checksums in .hex file (as warn if faulty input)
 *  - command line arguments (for enable/disable of checksum warning
 *                            and filename specification)
 *  - support for more chips and file formats
 *  - also write an assembler and programmer, perhaps even emulator
 */

#include <stdio.h>
#include <stdlib.h>

unsigned int len = 0;
unsigned int adress = 0;
unsigned int type = 0;
unsigned int line = 1;
unsigned int sum = 0;

unsigned char *data = NULL;

unsigned int datasize = 0;

void getdata(int c) {
	int ds2;
	if (c > datasize) {
		free(data);
		ds2 = ((c % 2048) + 1) * 2048;
		if (ds2 < datasize) {
			fprintf(stderr, "error error\n");
			exit(EXIT_FAILURE);
		}
		datasize = ds2;
		data = malloc(datasize);
		if (!data) {
			fprintf(stderr, "malloc() failed at line %d\n", line);
			exit(EXIT_FAILURE);
		}
	}
	if (fread((void *) data, 1, c, stdin) != c) {
		fprintf(stderr, "unexpected end of input at line %d\n", line);
		exit(EXIT_FAILURE);
	}
}

int dehex_map[256] = {
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  16, 16, 16, 16, 16, 16,
	16, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

int dehex(char *d, int c) {
	unsigned int r = 0, n;
	while (1) {
		n = dehex_map[*d];
		if (n & 16) {
			fprintf(stderr, "unexpected character at line %u\n", line);
			exit(EXIT_FAILURE);
		}
		r |= n;
		if (!--c)
			break;
		r <<= 4;
		++d;
	}
	return r;
}

typedef struct {
	const char *name;
	const unsigned int type;
} op_t;

enum {
	OT_DF,
	OT_BF,
	OT_F,
	OT_K8,
	OT_K11,
	OT_NA,
};

const int oplookup[] = {
	/* First two instructions handled in code later */
	0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
	15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16,
	17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18,
	19, 19, 19, 19, 20, 20, 20, 20, 21, 22, 23, 0,  24, 24, 24, 24,
};

op_t ops[] = {
	{ "[unknown]", OT_NA },
	{ "subwf", OT_DF },
	{ "decf", OT_DF },
	{ "iorwf", OT_DF },
	{ "andwf", OT_DF },
	{ "xorwf", OT_DF },
	{ "addwf", OT_DF },
	{ "movf", OT_DF },
	{ "comf", OT_DF },
	{ "incf", OT_DF },
	{ "decfsz", OT_DF },
	{ "rrf", OT_DF },
	{ "rlf", OT_DF },
	{ "swapf", OT_DF },
	{ "incfsz", OT_DF },
	{ "bcf", OT_BF }, /* 0x10 */
	{ "btfsc", OT_BF },
	{ "call", OT_K11 }, /* 0x20 */
	{ "goto", OT_K11 },
	{ "movlw", OT_K8 }, /* 0x30 */
	{ "retlw", OT_K8 },
	{ "iorlw", OT_K8 },
	{ "andlw", OT_K8 },
	{ "xorlw", OT_K8 },
	{ "sublw", OT_K8 },
};

const int oplookup0[] = {
	1,  0,  0,  0,  0,  0,  0,  0,  2,  3,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	1,  0,  0,  4,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	/* movwf = number of the beast */
};

op_t ops0[] = {
	{ "[unknown]", OT_NA },
	{ "nop", OT_NA },
	{ "return", OT_NA },
	{ "retfie", OT_NA },
	{ "sleep", OT_NA },
	{ "clrwdt", OT_NA },
	{ "movwf", OT_F },
};

int main(int argc, char *argv[]) {
	unsigned char *ptr;
	int i, c, o;
	op_t *oc;

	while (1) {
		linestart:
		getdata(1);
		if (*data != ':') {
			fprintf(stderr, "skipping line %u (no ':' at start)\n", line);
			while (*data != '\n')
				getdata(1);
			++line;
			goto linestart;
		}
		getdata(8);
		ptr = data;
		len = dehex(ptr, 2);
		ptr += 2;
		adress = dehex(ptr, 4);
		ptr += 4;
		type = dehex(ptr, 2);
		getdata((len << 1) + 2);
		ptr = data;

		/* should also add something to check checksum about here */

		for (i = 0; i < len; i += 2) {
			c = dehex(ptr, 2);
			ptr += 2;
			c |= (o = dehex(ptr, 2)) << 8;
			ptr += 2;
			if (o & 0xC0) {
				fprintf(stderr, "bad instruction at line %u", line);
				goto endline;
			}
			if (o)
				oc = ops + oplookup[o];
			else oc = ops0 + oplookup0[c & 0xFF];
			switch (oc->type) {
				case OT_DF:
					fprintf(stdout, "%04X (%04X): %s 0x%02X, %u\n", adress, c,
					        oc->name, c & 0x7F, (c & 0x80) >> 7);
					break;
				case OT_F:
					fprintf(stdout, "%04X (%04X): %s 0x%02X\n", adress, c,
					        oc->name, c & 0x7F);
					break;
				case OT_K8:
					fprintf(stdout, "%04X (%04X): %s 0x%02X\n", adress, c,
					        oc->name, c & 0xFF);
					break;
				case OT_K11:
					fprintf(stdout, "%04X (%04X): %s 0x%02X\n", adress, c,
					        oc->name, c & 0x7FF);
					break;
				case OT_BF: /* should we print bit # as hex or decimal? */
					fprintf(stdout, "%04X (%04X): %s 0x%02X, %u\n", adress, c,
					        oc->name, c & 0x7F, (c & 0x380) >> 7);
					break;
				default:
					fprintf(stdout, "%04X (%04X): %s\n", adress, c, oc->name);
			}
			++adress;
		}
		endline:
		if (type == 1)
			break;
		while (*data != '\n')
			getdata(1);
		++line;
	}
	return EXIT_SUCCESS;
}
