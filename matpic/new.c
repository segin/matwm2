/************************
 * generic string stuff *
 ************************/

#include <stdlib.h> /* NULL */

/* alfa[]
 *
 * description
 *   lookup table to figure what character is what sorta thing
 *   scroll down for meaning of numbers
 */
char alfa[256] = {
	32, 0,  0,  0,  0,  0,  0,  0,  0,  4,  16, 0,  0,  16, 0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  8,
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

/* symbols used in above lookup table "alfa[]" */
#define CT_LET 1  /* letter    (a-z, A-Z) */
#define CT_NUM 2  /* number    (0-9) */
#define CT_SPC 4  /* space     ('\t' or ' ') */
#define CT_SEP 8  /* separator ('_') */
#define CT_NL  16 /* newline   ('\r' or '\n') */
#define CT_NUL 32 /* nul/0     ('\0') */

/* skipsp(src)
 *
 * description
 *   advances *src until a character is found that is neither tab or space
 *
 * arguments
 *   char * src: zero terminated string
 *
 * return value
 *   int
 *   number of characters skipped
 */
int skipsp(char **src) {
	int n = 0;

	while (alfa[(unsigned char) **src] & CT_SPC)
		++n, ++(*src);
	return n;
}

/* skipnl(src)
 *
 * description
 *   advances *src until a non-newline character is found
 *
 * arguments
 *   char * src: zero terminated string
 *
 * return value
 *   int
 *   number of characters skipped
 */
int skipnl(char **src) {
	int n = 0;

	while (alfa[(unsigned char) **src] & CT_NL)
		++n, ++(*src);
	return n;
}

/* getid(src)
 *
 * description
 *   advances *src until a non-identifier character is found
 *   identifier characters matched like this regex "^[a-zA-Z_][a-zA-Z0-9_]*"
 *
 * arguments
 *   char * src: zero terminated string
 *
 * return value
 *   char *
 *   NULL if no identifier found
 *   original value of *src otherwise
 */
char *getid(char **src) {
	char *ret = NULL;

	if (alfa[(unsigned char) **src] & (CT_LET | CT_SEP)) {
		ret = *src;
		while (alfa[(unsigned char) **src] & (CT_LET | CT_SEP | CT_NUM))
			++(*src);
	}
	return ret;
}

/* cmpid(idl, idr)
 *
 * description
 *   compares two strings until the first non-identifier character
 *
 * arguments
 *   char *idl, *idr: 0 terminated strings to compare
 *
 * return value
 *   int
 *   amount of equal characters if all of them match
 *   otherwise 0
 */
int cmpid(char *idl, char *idr) {
	int n = 0;

	while (*idl && *idr &&
	       (alfa[(unsigned char) *idl] & (CT_LET | CT_SEP | CT_NUM)) &&
	       (alfa[(unsigned char) *idr] & (CT_LET | CT_SEP | CT_NUM)) &&
	       *(idl++) == *(idr++))
		++n;
	if(!(alfa[(unsigned char) *idl] & (CT_LET | CT_SEP | CT_NUM)) &&
	   !(alfa[(unsigned char) *idr] & (CT_LET | CT_SEP | CT_NUM)))
		return n;
	return 0;
}

char hexlookup[256] = {
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 16, 16, 16, 16, 16, 16,
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

int getnum(char **src, unsigned int *ret) {
	unsigned int c, r = 0;
	int n = 0, base = 10;

	if (**src == '0') {
		switch(*(++*src)) {
			case 'x':
				base = 16;
				break;
			case 'b':
				base = 2;
				break;
			case 'd':
				base = 10;
				break;
			case '0':
				base = 8;
				break;
			default:
				base = 8;
				goto octal0;
		}
		++*src;
		octal0:;
	}

	while ((c = hexlookup[**src]) != 16) {
		if (c >= base)
			goto endnum;
		r *= base;
		r += c;
		++*src, ++n;
	}
	endnum:
	*ret = r;
	return n;
}

/************************
 * generic memory stuff *
 ************************/

#include <stdlib.h> /* realloc(), exit(), NULL, EXIT_FAILURE */
#include <string.h> /* memcpy */
#include <stdio.h>  /* fprintf(), stderr */

#define BLOCK 2048  /* amount of memory to allocate in one go */
#define ARG_MAX 256 /* maximum number of arguments */

typedef struct {
	void *data;
	int size, count, space;
} arr_t;

void arr_new(arr_t *a, int size) {
	a->data = NULL;
	a->size = size;
	a->count = 0;
	a->space = 0;
}

void arr_add(arr_t *a, void *data) {
	if (a->space < (a->count + 1) * a->size) {
		a->data = realloc(a->data, (((a->size * a->count) % BLOCK) + 1) * BLOCK);
		if (a->data == NULL) {
			fprintf(stderr, "out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
	memcpy((void *) (((char *) a->data) + (a->size * a->count)), data, a->size);
	++(a->count);
}

void arr_free(arr_t *a) {
	free(a->data);
	a->data = NULL;
	a->size = 0;
	a->count = 0;
	a->space = 0;
}

/****************************
 * architecture definitions *
 ****************************/

typedef struct {
	char *instr;
	int opcode, atype, amask, imask;
} oc_t;

oc_t ocs14b[] = {
	{ "" },
};

void acmp14b(oc_t *oc, int *args) {

}

typedef struct {
	oc_t *ocs;
	void (*acmp)(oc_t *, int *);
} arch_t;

/*************
 * assembler *
 *************/

#include <stdlib.h> /* exit(), EXIT_FAILURE, EXIT_SUCCESS */
#include <stdio.h> /* fprintf(), stderr */

int line = 1;

typedef struct {
	int type, line;
	union {
		struct ins {
			char *args;
			int atype;
			oc_t *oc;
		};
		struct org {
			int address;
		};
		struct data {
			int value;
		};
	};
} ins_t;

enum itype {
	IT_END, /* end of data */
	IT_ORG, /* org directive */
	IT_DAT, /* data directive */
	IT_INS, /* an actual instruction */
};

arr_t inss;

typedef struct {
	char *name;
	int address;
} label_t;

arr_t labels;

void errexit(char *msg) {
	fprintf(stderr, "line %d: %s\n", line, msg);
	exit(EXIT_FAILURE);
}

unsigned int numarg(char **src);

unsigned int getval(char **src) {
	unsigned int val;

	/* FIXME implement NOT operator */
	skipsp(src);
	if (**src == '(') {
		++*src;
		val = numarg(src);
		if (**src != ')')
			errexit("can't find ')'");
		++*src;
	} else if(!getnum(src, &val)) {
		int i;
		label_t *label;
		char *id;

		id = getid(src);
		if (id == NULL)
			return val;

		/* get label or fail */
		for (i = 0; i < labels.count; ++i) {
			label = (label_t *) ((label_t *) labels.data) + i;
			if (cmpid(label->name, id))
				return label->address;
		}
		errexit("unknown identifier");
	}
	return val;
}

enum ops {
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_AND,
	OP_IOR,
	OP_EOR,
	OP_SHL,
	OP_SHR,
};

unsigned int numarg(char **src) {
	unsigned int lval, rval, op;

	lval = getval(src);

	while(1) {
		skipsp(src);
		switch (**src) {
			case '+':
				op = OP_ADD;
				++*src;
				break;
			case '-':
				op = OP_SUB;
				++*src;
				break;
			case '*':
				op = OP_MUL;
				++*src;
				break;
			case '/':
				op = OP_DIV;
				++*src;
				break;
			case '%':
				op = OP_MOD;
				++*src;
				break;
			case '&':
				op = OP_AND;
				++*src;
				break;
			case '|':
				op = OP_IOR;
				++*src;
				break;
			case '^':
				op = OP_EOR;
				++*src;
				break;
			case '<':
				if (*((*src) + 1) == '<') {
					op = OP_SHL;
					*src += 2;
					break;
				}
				return lval;
			case '>':
				if (*((*src) + 1) == '>') {
					op = OP_SHR;
					*src += 2;
					break;
				}
				return lval;
			default:
				return lval;
		}

		rval = getval(src);

		switch (op) {
			case OP_ADD:
				lval += rval;
				break;
			case OP_SUB:
				lval -= rval;
				break;
			case OP_MUL:
				lval *= rval;
				break;
			case OP_DIV:
				lval /= rval;
				break;
			case OP_MOD:
				lval %= rval;
				break;
			case OP_AND:
				lval &= rval;
				break;
			case OP_IOR:
				lval |= rval;
				break;
			case OP_EOR:
				lval ^= rval;
				break;
			case OP_SHL:
				lval <<= rval;
				break;
			case OP_SHR:
				lval >>= rval;
		}
	}
}

int getargs(char **src, int *args) {
	int n = 0;

	if (!*src)
		return 0;
	while (1) {
		args[n] = numarg(src);
		if (alfa[**src] & (CT_NUL | CT_NL))
			return n + 1;
		if (**src != ',')
			errexit("your argument is invalid");
		++n, ++*src;
		if (n == ARG_MAX)
			errexit("too many arguments");
	}
}

void assemble(char **code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));

	{ /* first pass */
		int address = 0;
		char *cur = NULL;
		label_t label;
		ins_t ins;
		int args[ARG_MAX];
		char *argp;

		while (**code) {
			 /* skip label and eat it if one is there
			 * also eat any preceding spaces before instruction
			 * return if nothing or nothing but label
			 */
			if (!skipsp(code)) {
				cur = getid(code);
				if (cur == NULL)
					errexit("malformed label");
				label.name = cur;
				label.address = address;
				arr_add(&labels, &label);
				if (!skipsp(code)) {
					if (!(alfa[**code] & (CT_NL | CT_NUL)))
						errexit("unexpected character within label");
					goto nextline;
				} else {
					if (alfa[**code] & (CT_NL | CT_NUL))
						goto nextline;
				}
			}

			/* eat the instruction (or directive) */
			cur = getid(code);
			if (cur == NULL)
				errexit("malformed instruction");

			/* check for arguments */
			argp = NULL;
			if (!skipsp(code)) {
				if (!(alfa[**code] & (CT_NL | CT_NUL)))
					errexit("unexpected character within instruction");
				goto nextline;
			} else {
				if (alfa[**code] & (CT_NL | CT_NUL))
					goto nextline;
				/* we got arguments */
				argp = *code;
				while (!(alfa[**code] & (CT_NUL | CT_NL)))
					++*code;
			}

			/* find instructon/directive */
			if (cmpid(cur, "org")) {
				if (getargs(&argp, args) != 1)
					errexit("invalid number of arguments to org directive");
				ins.type = IT_ORG;
				ins.line = line;
				ins.address = args[0];
				address = ins.address;
				arr_add(&inss, &ins);
				goto nextline;
			}
			if (cmpid(cur, "data")) {
				int i, n = getargs(&argp, args);
				for (i = 0; i < n; ++i) {
					ins.type = IT_DAT;
					ins.line = line;
					ins.value = args[i];
					arr_add(&inss, &ins);
				}
				goto nextline;
			}

			/* on to the next line */
			nextline:
			if (!skipnl(code) && **code)
				errexit("unexpected character");
			++line;
		}
	}
}

void cleanup(void) {
	arr_free(&inss);
	arr_free(&labels);
}

main() {
	char *code = " org 0x200 \ntest pest\r\n vest\nrest   \n data 1, 2, 3, test";

	assemble(&code);

	{
		int i;
		ins_t *ins;
		for (i = 0; i < inss.count; ++i) {
			ins = (ins_t *) ((ins_t *) inss.data) + i;
			printf("line %d, type %d:\n", ins->line, ins->type);
			switch (ins->type) {
				case IT_ORG:
					printf(" org 0x%X\n", ins->address);
					break;
				case IT_DAT:
					printf(" data 0x%X\n", ins->value);
					break;
				case IT_INS:
					printf(" opcode 0x%X", ins->oc->opcode);
			}
		}
	}

	cleanup();

	return EXIT_SUCCESS;
}
