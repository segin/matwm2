#include "host.h" /* exit(), EXIT_FAILURE, fprintf(), stderr */
#include "as.h" /* cnl (and other things) */
#include "dis.h"
#include "ppc.h"
#include "str.h" /* skipsp(), getnum(), getid(), cmpid(), alfa[] */
#include "misc.h"

char *infile = "<stdin>";
unsigned int address, line;
int dosnl = 0;
char linebuf[LBSIZE]; /* used by disassembler and preprocessor */

void cleanup(void) {
	arr_free(&inss);
	arr_free(&labels);
	arr_free(&dsym);
}

void reset(void) {
	infile = "<file>";
	cleanup();
}

void errexit(char *msg) {
	fprintf(stderr, "error: %s\n", msg);
	exit(EXIT_FAILURE);
}

void flerrexit(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: error: %s\n", file, line, msg);
	exit(EXIT_FAILURE);
}

void flwarn(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: warning: %s\n", file, line, msg);
}

void fawarn(char *file, int addr, char *msg) {
	fprintf(stderr, "%s: 0x%X: warning: %s\n", file, addr, msg);
}

void flmsg(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: message: %s\n", file, line, msg);
}

unsigned int getval(char **src) {
	unsigned int val;
	char *ns, *ne;

	skipsp(src);
	ns = *src;
	while (**src == '!' || **src == '~' || alfa[(unsigned char) **src] & CT_SPC)
		++*src;
	ne = *src;
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
		int i, j;
		label_t *label = (label_t *) labels.data;
		char *id;

		id = getid(src);
		if (id == NULL)
			goto gotval;

		/* get label or fail */
		for (i = 0; i < labels.count; ++i) {
			/* FIXME support label.local type format */
			if (cmpid(label[i].name, id)) {
				for (j = llbl; j >= 0; --j)
					if (label[j].local < label[i].local)
						break;
				if (label[i].parent != j)
					continue;
				val = label[i].address;
				goto gotval;
			}
		}
		aerrexit("unknown identifier");
	}
	gotval:
	while (ns != ne--) {
		if (*ne == '!')
			val = !val;
		else val = ~val;
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
	OP_LAND,
	OP_LOR,
	OP_LT,
	OP_GT,
	OP_LTE,
	OP_GTE,
	OP_EQ,
	OP_NE
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
				if (*((*src) + 1) == '&') {
					op = OP_LAND;
					*src += 2;
					break;
				}
				op = OP_AND;
				++*src;
				break;
			case '|':
				if (*((*src) + 1) == '|') {
					op = OP_LOR;
					*src += 2;
					break;
				}
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
				if (*((*src) + 1) == '=') {
					op = OP_LTE;
					*src += 2;
					break;
				}
				op = OP_LT;
				++*src;
				break;
			case '>':
				if (*((*src) + 1) == '>') {
					op = OP_SHR;
					*src += 2;
					break;
				}
				if (*((*src) + 1) == '=') {
					op = OP_GTE;
					*src += 2;
					break;
				}
				op = OP_GT;
				++*src;
				break;
			case '=':
				if (*((*src) + 1) == '=')
					++*src;
				op = OP_EQ;
				++*src;
				break;
			case '!':
				if (*((*src) + 1) == '=') {
					op = OP_NE;
					*src += 2;
					break;
				}
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
				break;
			case OP_LAND:
				lval = lval && rval;
				break;
			case OP_LOR:
				lval = lval || rval;
				break;
			case OP_LT:
				lval = lval < rval;
				break;
			case OP_LTE:
				lval = lval <= rval;
				break;
			case OP_GT:
				lval = lval > rval;
				break;
			case OP_GTE:
				lval = lval >= rval;
				break;
			case OP_EQ:
				lval = lval == rval;
				break;
			case OP_NE:
				lval = lval != rval;
				break;
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

	if (src == NULL || *src == NULL) {
		*word = NULL;
		return 0;
	}
	if (skipsp(src))
		prop |= WP_PSPC;
	if (**src == '.')
		prop |= WP_LOCAL | WP_LABEL;
	*word = getid(src);
	if (skipsp(src))
		prop |= WP_TSPC;
	if (**src == ':') {
		prop |= WP_LABEL | WP_TSPC; /* we don't require spaces after : */
		++*src;
	}
	return prop;
}

int egethex(char **s) {
	int n, r = 0, c = 2;
	while (c--) {
		r <<= 4;
		n = hexlookup[(unsigned char) **s];
		if (n == 16)
			return n;
		r |= n;
		++*s;
	}
	return n;
}

char *getstr(char **in) {
	string_t ret;
	char *p, b[5] = { 0, 0, 0, 0, 0 };
	int n;
	if (**in != '"')
		return NULL;
	vstr_new(&ret);
	++*in;
	p = *in;
	while (!(alfa[(unsigned char) *p] & (CT_NUL | CT_NL)) && *p != '"') {
		if (*p == '\\' && !(alfa[(unsigned char) p[1]] & (CT_NUL | CT_NL))) {
			vstr_addl(&ret, *in, p - *in);
			*in = p + 1;
			++p;
			switch (*p) {
				case 'a':
					vstr_add(&ret, "\x07");
					break;
				case 'b':
					vstr_add(&ret, "\x08");
					break;
				case 't':
					vstr_add(&ret, "\x09");
					break;
				case 'n':
					vstr_add(&ret, "\x0A");
					break;
				case 'v':
					vstr_add(&ret, "\x0B");
					break;
				case 'f':
					vstr_add(&ret, "\x0C");
					break;
				case 'r':
					vstr_add(&ret, "\x0D");
					break;
				case 'e':
					vstr_add(&ret, "\x1B");
					break;
				case 'U':
					++p;
					b[0] = egethex(&p);
					b[1] = egethex(&p);
					b[2] = egethex(&p);
					b[3] = egethex(&p);
					vstr_add(&ret, b);
					--p;
					break;
				case 'u':
					++p;
					b[0] = egethex(&p);
					b[1] = egethex(&p);
					b[2] = 0;
					vstr_add(&ret, b);
					--p;
					break;
				case 'x':
					++p;
					b[0] = egethex(&p);
					b[1] = 0;
					vstr_add(&ret, b);
					--p;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
					b[0] = ((*p) - 48);
					++p;
					if ((n = hexlookup[(unsigned char) *p]) < 8) {
						b[0] = (b[0] << 3) | n;
						++p;
						if ((n = hexlookup[(unsigned char) *p]) < 8)
							b[0] = (b[0] << 3) | n;
							++p;
					}
					b[1] = 0;
					vstr_add(&ret, b);
					--p;
					break;
				default:
					*b = *p;
					vstr_add(&ret, b);
			}
			++p;
			*in = p;
		} else ++p;
	}
	if (*p != '"')
		aerrexit("missing \"");
	vstr_addl(&ret, *in, p - *in);
	*in = p;
	++*in;
	return ret.data;
}
