#include "host.h" /* exit(), EXIT_FAILURE, fprintf(), stderr */
#include "as.h"
#include "dis.h"
#include "str.h" /* skipsp(), getnum(), getid(), cmpid(), alfa[] */
#include "misc.h"

char *infile = "<stdin>";
unsigned int address = 0, line = 1;

void cleanup(void) {
	arr_free(&inss);
	arr_free(&labels);
	arr_free(&dsym);
}

void reset(void) {
	line = 1;
	infile = "<file>";
	address = 0;
	arr_free(&inss);
	arr_free(&labels);
	/* reset disassembler too */
	arr_free(&dsym);
}

void errexit(char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

void flerrexit(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: %s\n", file, line, msg);
	exit(EXIT_FAILURE);
}

void flwarn(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: %s\n", file, line, msg);
	exit(EXIT_FAILURE);
}

void fawarn(char *file, int addr, char *msg) {
	fprintf(stderr, "%s: %X: %s\n", file, addr, msg);
}

unsigned int getval(char **src) {
	unsigned int val;
	int not = 0;

	skipsp(src);
	while (**src == '!' || **src == '~')
		++not, ++*src;
	skipsp(src);
	if (**src == '(') {
		++*src;
		val = numarg(src);
		if (**src != ')')
			aerrexit("missing ')'");
		++*src;
	} else if (**src == '$') {
		val = address;
		++*src;
	} else if(!getnum(src, &val)) {
		int i;
		label_t *label;
		char *id;

		id = getid(src);
		if (id == NULL)
			goto gotval;

		/* get label or fail */
		for (i = 0; i < labels.count; ++i) {
			label = (label_t *) ((label_t *) labels.data) + i;
			if (cmpid(label->name, id)) {
				val = label->address;
				goto gotval;
			}
		}
		aerrexit("unknown identifier");
	}
	gotval:
	if (not)
		val = ~val;
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
	OP_SHR
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

	if (*src == NULL)
		return 0;
	while (1) {
		args[n] = numarg(src);
		if (alfa[(unsigned char) **src] & (CT_NUL | CT_NL))
			return n + 1;
		if (**src != ',')
			aerrexit("your argument is invalid");
		++n, ++*src;
		if (n == ARG_MAX)
			aerrexit("too many arguments");
	}
}

int getword(char **src, char **word) {
	int prop = 0;

	if (skipsp(src))
		prop |= WP_PSPC;
	if (**src == '.') {
		prop |= WP_LOCAL;
		++*src;
	}
	*word = getid(src);
	if (skipsp(src))
		prop |= WP_TSPC;
	if (**src == ':') {
		prop |= WP_LABEL | WP_TSPC; /* we don't require spaces after : */
		++*src;
	}
	return prop;
}

