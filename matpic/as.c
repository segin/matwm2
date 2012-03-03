/*************
 * assembler *
 *************/

#include <stdlib.h> /* NULL */
#include <string.h> /* memcpy() */
#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"
#include "misc.h" /* countargs(), getargs(), getstr(), file, address, line */

int llbl, count;
unsigned int addrl;

arr_t inss = { NULL, 0, 0, 0 }; /* these need to be 0 so cleanup() before assemble won't fail */
arr_t labels = { NULL, 0, 0, 0 };

char *lp, *ip, *argp, *nextln;
int pspc, tspc, prefix, run;

int getprefix(char **src) {
	char *p = *src;
	int n = 0;
	skipsp(&p);
	while (ctype(*p) & (CT_PPC))
		++p, ++n;
	if (n) {
		*src = p;
		skipsp(src);
	}
	return n;
}

int parseln(char *in) {
	if (!*in)
		return 0;
	if (run == 0) {
		int r = 1;
		pspc = skipsp(&in);
		prefix = getprefix(&in);
		lp = NULL;
		ip = getid(&in);
		argp = NULL;
		if (*in == ':') {
			lp = ip;
			++in;
			skipsp(&in);
			ip = getid(&in);
		}
		if (ip == NULL) {
			if (!(ctype(*in) & (CT_NL | CT_NUL)))
				r = 2;
			goto endln;
		}
		tspc = skipsp(&in);
		if (!(ctype(*in) & (CT_NL | CT_NUL))) {
			if (tspc)
				argp = in;
			else {
				ip = NULL;
				r = 2;
				goto endln;
			}
		}
		endln:
		while (!(ctype(*in) & (CT_NUL | CT_NL)))
			++in;
		skipnl(&in);
		nextln = in;
		++run;
		return r;
	}
	run = 0;
	if (lp == NULL && ip != NULL && !pspc && !prefix) { /* ip possibly a label */
		lp = ip;
		ip = NULL;
		if (argp != NULL) {
			in = argp;
			argp = NULL;
			ip = getid(&in);
			tspc = skipsp(&in);
			if (ip == NULL) {
				if (!(ctype(*in) & (CT_NL | CT_NUL)))
					return 2;
				return 1;
			}
			if (!(ctype(*in) & (CT_NL | CT_NUL))) {
				if (tspc) {
					argp = in;
					return 1;
				}
				return 2;
			}
		}
		return 1;
	}
	lp = NULL; /* to be sure we don't do twice the same thing */
	ip = NULL;
	return 3;
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
		if (arr_item(labels, label_t, i)->local < label.local) {
			label.parent = i;
			break;
		}
	}
	for (i = 0; i < labels.count; ++i) { /* check if already exists */
		li = arr_item(labels, label_t, i);
		if (cmpid(li->name, label.name) && label.parent == li->parent && label.local == li->local)
			flerrexit("duplicate label");
	}
	arr_add(&labels, &label);
	ins.type = IT_LBL;
	ins.d.lbl.lbl = labels.count - 1;
	arr_add(&inss, &ins);
}

int insfind(char *ip, char *argp) {
	int args[ARG_MAX];
	oc_t *oc = arch->ocs;
	ins_t ins;

	ins.line = line;
	if (cmpid(ip, "org")) {
		getargs(argp, args, 1, 1);
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
		getargs(argp, args, 1, 1);
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
	if (prefix)
		return 0;
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
		getargs(argp, args, 1, 1);
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
		int r;

		file = infile;
		line = 1;
		address = 0;
		llbl = -1;
		count = 1;
		run = 0;
		while ((r = parseln(code))) {
			ins.line = line;
			addrl = address; /* case there is a label we have the address at start of line */
			if (r == 2)
				flerrexit("syntax error");
			if (lp != NULL)
				addlabel(lp);
			if (ip != NULL) {
				if (insfind(ip, argp))
					run = 0;
				else if (run == 0)
					flerrexit("no such instruction or directive");
			}
			if (run == 0) {
				if (count)
					++line;
				code = nextln;
			}
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
					c = getargs(ins->d.ins.args, args, 0, ARG_MAX);
					arch->acmp(ins->d.ins.oc, ins->d.ins.atype, c, args);
					++address;
					break;
				case IT_ORG:
					address = ins->d.org.address;
					break;
				case IT_DAT:
					c = getargs(ins->d.data.args, args, 1, ARG_MAX);
					for (i = 0; i < c; ++i) {
						ins->d.data.value = args[i];
						++ins;
					}
					if (i)
						--ins;
					address += (arch->dlen / arch->align) * c;
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
