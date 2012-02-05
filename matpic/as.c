/*************
 * assembler *
 *************/

#include <stdlib.h> /* exit(), EXIT_FAILURE */
#include <stdio.h> /* fprintf(), stderr */

#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"

int line = 1;
char file[FN_MAX] = "<file>";
int address = 0;

arr_t inss;
arr_t labels;

void errexit(char *msg) {
	fprintf(stderr, "%s: line %d: %s\n", file, line, msg);
	exit(EXIT_FAILURE);
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
			errexit("can't find ')'");
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
		errexit("unknown identifier");
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

void setfile(char *fn) {
	int i;
	for (i = 0; i < FN_MAX - 1; ++i) {
		if (alfa[fn[i]] & (CT_NUL | CT_NL))
			break;
		file[i] = fn[i];
	}
	file[i] = 0;
	line = 0; /* will be incremented soon enough */
}

void assemble(char **code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));

	{ /* first pass */
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
			if (alfa[**code] & (CT_NL | CT_NUL))
				goto nextline;

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
			ins.line = line;
			if (cmpid(cur, "org")) {
				if (getargs(&argp, args) != 1)
					errexit("invalid number of arguments to org directive");
				ins.type = IT_ORG;
				ins.address = args[0];
				address = ins.address;
				arr_add(&inss, &ins);
				goto nextline;
			}
			if (cmpid(cur, "data")) {
				int i, n = getargs(&argp, args);
				for (i = 0; i < n; ++i) {
					ins.type = IT_DAT;
					ins.value = args[i];
					arr_add(&inss, &ins);
				}
				address += n;
				goto nextline;
			}
			if (cmpid(cur, "file")) {
				if (argp == NULL)
					errexit("'file' directive needs an argument");
				setfile(argp);
				goto nextline;
			}
			if (cmpid(cur, "line")) {
				if (getargs(&argp, args) != 1)
					errexit("'line' directive wants exactly 1 argument");
				line = args[0] - 1;
			}

			{ /* not a directive, we'll try to find an instruction then */
				oc_t *oc = arch->ocs;
				while (oc->name != NULL) {
					if (cmpid(cur, oc->name)) {
						ins.type = IT_INS;
						ins.oc = oc->oc;
						ins.atype = oc->atype;
						ins.args = argp;
						arr_add(&inss, &ins);
						++address;
						goto nextline;
					}
					++oc;
				}
				errexit("unknown instruction");
			}

			/* on to the next line */
			nextline:
			if (!skipnl(code) && **code)
				errexit("unexpected character");
			++line;
		}
		ins.type = IT_END;
		arr_add(&inss, &ins);
	}
	{ /* second pass */
		ins_t *ins = (ins_t *) inss.data;
		int c, args[ARG_MAX];

		while (ins->type != IT_END) {
			if (ins->type == IT_INS) {
				c = getargs(&(ins->args), args);
				ins->oc = arch->acmp(ins->oc, ins->atype, c, args);
			}
			++ins;
		}
	}
}

void cleanup(void) {
	arr_free(&inss);
	arr_free(&labels);
}
