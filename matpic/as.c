/*************
 * assembler *
 *************/

#include "host.h" /* memcpy() */
#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"
#include "misc.h" /* flerrexit(), getargs() */
#include "main.h" /* infile, address, line */

char file[FN_MAX];

arr_t inss = { NULL, 0, 0, 0 };
arr_t labels = { NULL, 0, 0, 0 };

void aerrexit(char *msg) {
	flerrexit(file, line, msg);
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

