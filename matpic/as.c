/*************
 * assembler *
 *************/

#include "host.h" /* memcpy() */
#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"
#include "misc.h" /* flerrexit() */
#include "main.h" /* infile, address, line */

char file[FN_MAX];

arr_t inss = { NULL, 0, 0, 0 };
arr_t labels = { NULL, 0, 0, 0 };

void aerrexit(char *msg) {
	flerrexit(file, line, msg);
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
			aerrexit("can't find ')'");
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

	if (!*src)
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

int countargs(char *src) {
	int n = 1;
	if (!src)
		return 0;
	while(!(alfa[(unsigned char) *src] & (CT_NUL | CT_NL))) {
		if (*src == ',')
			++n;
		++src;
	}
	return n;
}

void setfile(char *fn) {
	int i;
	if (*fn != '"')
		aerrexit("argument to file directive needs double quotes");
	++fn;
	for (i = 0; i < FN_MAX - 1; ++i) {
		if (alfa[(unsigned char) fn[i]] & (CT_NUL | CT_NL))
			aerrexit("missing double quote");
		if (fn[i] == '"')
			break;
		file[i] = fn[i];
	}
	fn += i + 1;
	skipsp(&fn);
	if (!(alfa[(unsigned char) *fn] & (CT_NUL | CT_NL)))
		aerrexit("invalid crap after file directive");
	file[i] = 0;
	line = 0; /* will be incremented soon enough */
}

void initfile(void) {
	int i;
	for (i = 0; i < FN_MAX && infile[i]; ++i)
		file[i] = infile[i];
}

void assemble(char *code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));

	{ /* first pass */
		char *cur = NULL;
		label_t label;
		ins_t ins;
		int args[ARG_MAX];
		char *argp;
		label_t *li;
		int i;

		initfile();

		while (*code) {
			 /* skip label and eat it if one is there
			 * also eat any preceding spaces before instruction
			 * return if nothing or nothing but label
			 */
			if (!skipsp(&code)) {
				cur = getid(&code);
				if (cur == NULL) {
					if (alfa[(unsigned char) *code] & (CT_NL | CT_NUL))
						goto nextline;
					aerrexit("malformed label");
				}
				label.name = cur;
				label.address = address / arch->align;
				for (i = 0; i < labels.count; ++i) { /* check if already exists */
					li = (label_t *) ((label_t *) labels.data) + i;
					if (cmpid(li->name, cur))
						aerrexit("duplicate label");
				}
				arr_add(&labels, &label);
				if (!skipsp(&code)) {
					if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL)))
						aerrexit("unexpected character within label");
					goto nextline;
				} else {
					if (alfa[(unsigned char) *code] & (CT_NL | CT_NUL))
						goto nextline;
				}
			}
			if (alfa[(unsigned char) *code] & (CT_NL | CT_NUL))
				goto nextline;

			/* eat the instruction (or directive) */
			cur = getid(&code);
			if (cur == NULL)
				aerrexit("malformed instruction");

			/* check for arguments */
			argp = NULL;
			if (!skipsp(&code)) {
				if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL)))
					aerrexit("unexpected character within instruction");
			} else {
				if (alfa[(unsigned char) *code] & (CT_NL | CT_NUL))
					goto nextline;
				/* we got arguments */
				argp = code;
				while (!(alfa[(unsigned char) *code] & (CT_NUL | CT_NL)))
					++code;
			}

			/* find instructon/directive */
			ins.line = line;
			if (cmpid(cur, "org")) {
				if (getargs(&argp, args) != 1)
					aerrexit("invalid number of arguments to org directive");
				ins.type = IT_ORG;
				ins.d.org.address = args[0] * arch->align;
				address = ins.d.org.address;
				arr_add(&inss, &ins);
				goto nextline;
			}
			if (cmpid(cur, "data")) {
				int n = countargs(argp);
				ins.type = IT_DAT;
				ins.d.data.args = argp;
				address += n * arch->align;
				for (; n > 0; --n)
					arr_add(&inss, &ins);
				goto nextline;
			}
			if (cmpid(cur, "file")) {
				if (argp == NULL)
					aerrexit("'file' directive needs an argument");
				setfile(argp);
				ins.type = IT_FIL;
				ins.d.file.file = argp;
				arr_add(&inss, &ins);
				goto nextline;
			}
			if (cmpid(cur, "line")) {
				if (getargs(&argp, args) != 1)
					aerrexit("'line' directive wants exactly 1 argument");
				line = args[0] - 1;
				goto nextline;
			}

			{ /* not a directive, we'll try to find an instruction then */
				oc_t *oc = arch->ocs;
				while (oc->name != NULL) {
					if (cmpid(cur, oc->name)) {
						ins.type = IT_INS;
						memcpy((void *) ins.d.ins.oc, (void *) oc->oc, sizeof(oc->oc));
						ins.d.ins.len = oc->len;
						ins.d.ins.atype = oc->atype;
						ins.d.ins.args = argp;
						arr_add(&inss, &ins);
						address += ins.d.ins.len;
						goto nextline;
					}
					++oc;
				}
				aerrexit("unknown instruction");
			}

			/* on to the next line */
			nextline:
			if (!skipnl(&code) && *code)
				aerrexit("unexpected character");
			++line;
		}
		ins.type = IT_END;
		arr_add(&inss, &ins);
	}
	{ /* second pass */
		ins_t *ins = (ins_t *) inss.data;
		int i, c, args[ARG_MAX];

		initfile();
		address = 0;

		while (ins->type != IT_END) {
			line = ins->line;
			switch (ins->type) {
				case IT_INS:
					c = getargs(&(ins->d.ins.args), args);
					arch->acmp(ins->d.ins.oc, ins->d.ins.atype, c, args);
					++address;
					break;
				case IT_ORG:
					address = ins->d.org.address;
					break;
				case IT_DAT:
					c = getargs(&(ins->d.data.args), args);
					for (i = 0; i < c; ++i) {
						ins->d.data.value = args[i];
						++ins;
					}
					if (i)
						--ins;
					address += c;
					break;
				case IT_FIL:
					setfile(ins->d.file.file);
					break;
			}
			++ins;
		}
	}
}

