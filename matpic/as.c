/*************
 * assembler *
 *************/

#include <stdlib.h> /* NULL */
#include <string.h> /* memcpy() */
#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"
#include "misc.h" /* getargs(), clearfile(), getstr(), file, address, line */

int llbl, count;
unsigned int addrl;

arr_t inss = { NULL, 0, 0, 0 };
arr_t labels = { NULL, 0, 0, 0 };

int countargs(char *src) {
	int n = 1;
	if (src == NULL)
		return 0;
	while(!(ctype(*src) & (CT_NUL | CT_NL))) {
		if (*src == ',')
			++n;
		++src;
	}
	return n;
}

void addlabel(char *lp) {
	label_t label, *li;
	ins_t ins;
	int i;
	/* we has a lebel */
	for (label.local = 0; *lp == '.'; ++label.local, ++lp);
	label.name = lp;
	label.address = addrl / arch->align;
	label.parent = -1;
	for (i = labels.count - 1; i >= 0; --i) { /* find owner */
		li = (label_t *) ((label_t *) labels.data) + i;
		if (li->local < label.local) {
			label.parent = i;
			break;
		}
	}
	for (i = 0; i < labels.count; ++i) { /* check if already exists */
		li = (label_t *) ((label_t *) labels.data) + i;
		if (cmpid(li->name, label.name) && label.parent == li->parent && label.local == li->local)
			flerrexit("duplicate label");
	}
	arr_add(&labels, &label);
	ins.type = IT_LBL;
	ins.d.lbl.lbl = labels.count - 1;
	arr_add(&inss, &ins);
}

int insfind(char *lp, char *ip, char *argp) {
	int args[ARG_MAX];
	oc_t *oc = arch->ocs;
	ins_t ins;

	ins.line = line;
	if (cmpid(ip, "org")) {
		if (getargs(&argp, args) != 1)
			flerrexit("invalid number of arguments to org directive");
		ins.type = IT_ORG;
		ins.d.org.address = args[0] * arch->align;
		address = ins.d.org.address;
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "file")) {
		if (argp == NULL)
			flerrexit("'file' directive needs an argument");
		count = 1;
		line = 0; /* will be incremented soon enough */
		ins.type = IT_FIL;
		ins.d.file.file = argp;
		arr_add(&inss, &ins);
		if (file != infile)
			free(file);
		file = getstr(&argp, 0);
		if (file == NULL)
			errexit("syntax error on file directive");
		if (!(ctype(*argp) & (CT_NUL | CT_NL)))
			flerrexit("invalid data after file directive");
		return 1;
	}
	if (cmpid(ip, "line")) {
		if (getargs(&argp, args) != 1)
			flerrexit("'line' directive wants exactly 1 argument");
		count = 1;
		line = args[0] - 1;
		return 1;
	}
	if (cmpid(ip, "nocount")) {
		if (argp != NULL)
			flerrexit("too many arguments for nocount directive");
		count = 0;
		return 1;
	}
	if (cmpid(ip, "data")) {
		int n = countargs(argp);
		ins.type = IT_DAT;
		ins.d.data.args = argp;
		address += n * arch->align;
		for (; n > 0; --n)
			arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "equ")) {
		if (getargs(&argp, args) != 1)
			flerrexit("'equ' directive wants exactly 1 argument");
		if (labels.count == 0)
			flerrexit("'equ' before label");
		((label_t *) labels.data + labels.count - 1)->address = args[0];
		return 1;
	}

	while (oc->name != NULL) {
		if (cmpid(ip, oc->name)) {
			ins.type = IT_INS;
			memcpy((void *) ins.d.ins.oc, (void *) oc->oc, sizeof(oc->oc));
			ins.d.ins.len = oc->len;
			ins.d.ins.atype = oc->atype;
			ins.d.ins.args = argp;
			arr_add(&inss, &ins);
			address += ins.d.ins.len;
			return 1;
		}
		++oc;
	}
	return 0;
}

void assemble(char *code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));

	{ /* first pass */
		ins_t ins;
		char *lp, *ip, *argp;
		int wp;

		file = infile;
		line = 1;
		address = 0;
		llbl = -1;
		count = 1;
		while (*code) {
			ins.line = line;
			addrl = address; /* case there is a label we have the address at start of line */
			lp = NULL;
			argp = NULL;
			wp = getword(&code, &ip);
			if (wp & (WP_LABEL | WP_LOCAL)) { /* we have label and we're sure about it */
				if (!(wp & WP_TSPC) && !(ctype(*code) & (CT_NL | CT_NUL)))
					flerrexit("invalid character in local label"); /* can only happen with local label */
				lp = ip;
				addlabel(lp);
				wp = getword(&code, &ip);
			}
			if (ip == NULL) {
				if (!(ctype(*code) & (CT_NL | CT_NUL)))
					flerrexit("invalid identifier");
				goto endln;
			}
			if (!(ctype(*code) & (CT_NL | CT_NUL))) {
				if (wp & WP_TSPC)
					argp = code;
				else flerrexit("invalid identifier");
			}
			if (insfind(lp, ip, argp))
				goto endln;
			if (lp == NULL) {
				if (!(wp & WP_PSPC)) {
					lp = ip;
					addlabel(lp);
					wp = getword(&code, &ip);
					argp = NULL;
					if (ip == NULL) {
						if (!(ctype(*code) & (CT_NL | CT_NUL)))
							flerrexit("invalid identifier");
						goto endln;
					}
					if (!(ctype(*code) & (CT_NL | CT_NUL))) {
						if (wp & WP_TSPC)
							argp = code;
						else flerrexit("invalid identifier");
					}
					if (!insfind(lp, ip, argp))
						flerrexit("no such instruction/directive");
				} else flerrexit("no such instruction/directive");
			}
			endln:
			while (!(ctype(*code) & (CT_NUL | CT_NL)))
				++code;
			skipnl(&code);
			if (count)
				++line;
		}
		ins.type = IT_END;
		arr_add(&inss, &ins);
	}
	{ /* second pass */
		ins_t *ins = (ins_t *) inss.data;
		int i, c, args[ARG_MAX];
		char *s;

		file = infile;
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
					if (file != infile)
						free(file);
					s = ins->d.file.file;
					file = getstr(&s, 1);
					break;
				case IT_LBL:
					llbl = ins->d.lbl.lbl;
					break;
			}
			++ins;
		}
	}
	llbl = -1; /* important */
	if (file != infile)
		free(file);
}
