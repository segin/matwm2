#include <stdlib.h> /* NULL, exit(), EXIT_FAILURE, malloc() */
#include <string.h> /* strlen() */
#include <stdarg.h>
#include "as.h" /* inss, labels, llbl */
#include "dis.h" /* dsym */
#include "str.h" /* skipsp(), getnum(), getid(), cmpid(), ctype(), lower[] */
#include "misc.h"
#include "lineno.h"

char *infile = "<stdin>";

unsigned long address;

void cleanup(void) {
	arr_free(&inss);
	arr_free(&labels);
	arr_free(&dsym);
	vstr_free(&outbuf);
}

void vaflwarn(char *pro, char *fmt, va_list ap) {
	mfprintf(mstderr, pro, lineno_getfile(), lineno_get());
	mvafprintf(mstderr, fmt, ap);
	mfprint(mstderr, "\n");
	lineno_printorigin(mstderr);
}

void errexit(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	mfprint(mstderr, "error: ");
	mvafprintf(mstderr, fmt, ap);
	mfprint(mstderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

void flerrexit(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vaflwarn("%s: line %u: error: ", fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void flwarn(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vaflwarn("%s: line %u: warning: ", fmt, ap);
	va_end(ap);
}

void flmsg(char *msg) {
	mfprintf(mstderr, "%s: line %u: message: %s\n", lineno_getfile(), lineno_get(), msg);
}

char *readfile(char *path) {
	int pos = 0, mem = 0, r = 0;
	ioh_t *infd = mstdin;
	char *ret = NULL;
	if (path != NULL) {
		infd = mfopen(path, MFM_RD);
		if (infd == NULL)
			return ret;
	}
	if (infd == NULL)
		errexit("failed to open file \"%s\"", path);
	do {
		pos += r;
		if (mem < pos + BLOCK) {
			if (mem + BLOCK < mem)
				errexit("wtf integer overflow");
			mem += BLOCK;
			ret = (char *) realloc((void *) ret, mem + 1); /* + 1 for ending 0 */
			if (ret == NULL)
				errexit("out of memory");
		}
	} while((r = mfread(infd, ret + pos, BLOCK)) > 0);
	ret[pos] = 0;
	if (path != NULL)
		mfclose(infd);
	return ret;
}

int cmplid(char **idlp, char *idr) {
	char *idl = *idlp;

	while (*idl && *idr && *idl != '.' &&
	       (ctype(*idl) & (CT_LET | CT_SEP | CT_NUM)) &&
	       (ctype(*idr) & (CT_LET | CT_SEP | CT_NUM)) &&
	       lower[(unsigned char) *idl] == lower[(unsigned char) *idr])
		++idl, ++idr;
	if((!(ctype(*idl) & (CT_LET | CT_SEP | CT_NUM)) || *idl == '.') &&
	   !(ctype(*idr) & (CT_LET | CT_SEP | CT_NUM))) {
		if (*idl == '.') {
			*idlp = idl;
			return 2;
		}
		return 1;
	}
	return 0;
}

unsigned long getval(char **src) {
	unsigned long val;
	char *ns, *ne;

	skipsp(src);
	ns = *src;
	while (**src == '!' || **src == '~' || ctype(**src) & CT_SPC)
		++*src;
	ne = *src;
	skipsp(src);
	if (**src == '(') {
		++*src;
		val = numarg(src);
		if (**src != ')')
			flerrexit("missing ')'");
		++*src;
		goto gotval;
	}
	if(getnum(src, &val))
		goto gotval;
	if (**src == '$') {
		val = address;
		++*src;
		goto gotval;
	}
	{
		int i, j, r, l = -1, local = 0;
		label_t *label = (label_t *) labels.data;
		char *id;
		id = getid(src);
		if (id == NULL)
			flerrexit("constant expected");
			/* get label or fail */
		while (*id == '.')
			++id, ++local;
		for (i = 0; i < labels.count; ++i) {
			if (cmpid(id, label[i].name) && label[i].local == local) {
				for (j = llbl; j >= 0; --j)
					if (label[j].local < label[i].local)
						break;
				if (label[i].parent != j)
					continue;
			} else continue;
			val = label[i].address;
			goto gotval;
		}
		for (i = 0; i < labels.count; ++i) {
			if (l != -1 && label[i].local < l)
				break;
			if ((r = cmplid(&id, label[i].name))) {
				if (l == -1) {
					if (label[i].local != local)
						continue;
					for (j = llbl; j >= 0; --j)
						if (label[j].local < label[i].local)
							break;
					if (label[i].parent != j)
						continue;
				}
				if (r & 2) {
					while (*id == '.')
						++id, ++local;
					l = label[i].local;
					continue;
				}
				val = label[i].address;
				goto gotval;
			}
		}
		flerrexit("unknown identifier '%s'", mstrldup(id, idlen(id)));
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
	OP_NOP,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_ADD,
	OP_SUB,
	OP_SHL,
	OP_SHR,
	OP_AND,
	OP_EOR,
	OP_IOR,
	OP_LT,
	OP_GT,
	OP_LTE,
	OP_GTE,
	OP_EQ,
	OP_NE,
	OP_LAND,
	OP_LOR
};

unsigned long calc(int op, unsigned long lval, unsigned long rval) {
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
			if (!rval)
				flerrexit("division by zero");
			lval /= rval;
			break;
		case OP_MOD:
			if (!rval)
				flerrexit("division by zero");
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
	return lval;
}

#define CALC_MAX 512

void _calc(unsigned long *lval, unsigned long *op, int len, int pre_min, int pre_max) {
	int i, j;
	for (i = 0, j = 0; i < len; ++i) {
		if (op[i] >= pre_min && op[i] <= pre_max)
			lval[j] = calc(op[i], lval[j], lval[i + 1]);
		else if (op[i] > pre_max)
			j = i + 1;
	}
}

unsigned long numarg(char **src) {
	unsigned int i;
	unsigned long lval[CALC_MAX], op[CALC_MAX];

	for (i = 0; i < CALC_MAX; ++i) {
		lval[i] = getval(src);
		skipsp(src);
		switch (**src) {
			case '+':
				op[i] = OP_ADD;
				++*src;
				break;
			case '-':
				op[i] = OP_SUB;
				++*src;
				break;
			case '*':
				op[i] = OP_MUL;
				++*src;
				break;
			case '/':
				op[i] = OP_DIV;
				++*src;
				break;
			case '%':
				op[i] = OP_MOD;
				++*src;
				break;
			case '&':
				if (*((*src) + 1) == '&') {
					op[i] = OP_LAND;
					*src += 2;
					break;
				}
				op[i] = OP_AND;
				++*src;
				break;
			case '|':
				if (*((*src) + 1) == '|') {
					op[i] = OP_LOR;
					*src += 2;
					break;
				}
				op[i] = OP_IOR;
				++*src;
				break;
			case '^':
				op[i] = OP_EOR;
				++*src;
				break;
			case '<':
				if (*((*src) + 1) == '<') {
					op[i] = OP_SHL;
					*src += 2;
					break;
				}
				if (*((*src) + 1) == '=') {
					op[i] = OP_LTE;
					*src += 2;
					break;
				}
				op[i] = OP_LT;
				++*src;
				break;
			case '>':
				if (*((*src) + 1) == '>') {
					op[i] = OP_SHR;
					*src += 2;
					break;
				}
				if (*((*src) + 1) == '=') {
					op[i] = OP_GTE;
					*src += 2;
					break;
				}
				op[i] = OP_GT;
				++*src;
				break;
			case '=':
				if (*((*src) + 1) == '=')
					++*src;
				op[i] = OP_EQ;
				++*src;
				break;
			case '!':
				if (*((*src) + 1) == '=') {
					op[i] = OP_NE;
					*src += 2;
					break;
				}
			default:
				op[i] = OP_NOP;
				goto end;
		}
	}
	end:
	if (i == CALC_MAX)
		flerrexit("constant too complex");

	_calc(lval, op, i, OP_MUL, OP_MOD);
	_calc(lval, op, i, OP_ADD, OP_SUB);
	_calc(lval, op, i, OP_SHL, OP_SHR);
	_calc(lval, op, i, OP_AND, OP_AND);
	_calc(lval, op, i, OP_EOR, OP_EOR);
	_calc(lval, op, i, OP_IOR, OP_IOR);
	_calc(lval, op, i, OP_LT, OP_GTE);
	_calc(lval, op, i, OP_EQ, OP_NE);
	_calc(lval, op, i, OP_LAND, OP_LAND);
	_calc(lval, op, i, OP_LOR, OP_LOR);

	return lval[0];
}

int getargs(char *src, int *args, int min, int max) {
	int n = 0;

	if (src != NULL)
		while (1) {
			args[n++] = numarg(&src);
			if (ctype(*src) & (CT_NUL | CT_NL))
				break;
			if (*src != ',')
				flerrexit("your argument is invalid");
			++src;
			if (n == ARG_MAX)
				flerrexit("too many arguments");
			skipsp(&src);
			if (ctype(*src) & (CT_NUL | CT_NL))
				flerrexit("expression expected");
		}
	if (n < min)
		flerrexit("too few arguments");
	if (n > max)
		flerrexit("too many arguments");
	return n;
}

void parseargs(char *in, char *mode, ...) {
	int *i;
	char **s;
	va_list ap;
	if (in == NULL)
		flerrexit("too few arguments");
	va_start(ap, mode);
	while (1) {
		switch (*mode) {
			case 'n':
				i = va_arg(ap, int *);
				*i = numarg(&in);
				break;
			case 's':
				s = va_arg(ap, char **);
				*s = getstr(&in);
				if (*s == NULL)
					flerrexit("your argument is invalid");
				break;
			case 'i':
				s = va_arg(ap, char **);
				*s = getid(&in);
				if (*s == NULL)
					flerrexit("your argument is invalid");
				skipsp(&in);
				break;
		}
		if (mode[1] == '+') {
			if (*in == ',') {
				++in;
				skipsp(&in);
				continue;
			} else break;
		}
		if (*++mode) {
			if (*in != ',')
				flerrexit("too few arguments");
			++in;
			skipsp(&in);
		} else break;
	}
	if (*in == ',')
		flerrexit("too many arguments");
	if (!(ctype(*in) & (CT_NL | CT_NUL)))
		flerrexit("your argument is invalid");
	va_end(ap);
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

int sclen(char *in) {
	char end = *in++;
	int ret = 0;
	while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != end && *(in - 1) != '\\')
		++ret;
	if (*in != end)
		flerrexit("missing '%c'", end);
	return ret;
}

char *getstr(char **in) {
	string_t ret;
	char end, *p;
	if (**in != '"' && **in != '\'')
		return NULL;
	end = **in;
	vstr_new(&ret);
	++*in;
	p = *in;
	while (!(ctype(*p) & (CT_NUL | CT_NL))) {
		while (!(ctype(*p) & (CT_NUL | CT_NL)) && *p != end)
			++p;
		vstr_addl(&ret, *in, p - *in);
		*in = p;
		if (*p == end && *(p - 1) != '\\')
			break;
	}
	if (*p != end)
		flerrexit("missing '%c'", end);
	*in = p;
	++*in;
	skipsp(in);
	return ret.data;
}

char *unescape(char *in) {
	string_t ret;
	char end, *p, b[5] = { 0, 0, 0, 0, 0 };
	int n;
	end = *in;
	vstr_new(&ret);
	p = in;
	while (*p) {
		if (*p == '\\' && !(ctype(p[1]) & (CT_NUL | CT_NL))) {
			vstr_addl(&ret, in, p - in);
			in = p + 1;
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
			in = p;
		} else ++p;
	}
	vstr_addl(&ret, in, p - in);
	in = p;
	++in;
	skipsp(&in);
	return ret.data;	
}
