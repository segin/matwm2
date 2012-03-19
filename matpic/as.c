/*************
 * assembler *
 *************/

#include <stdlib.h> /* NULL */
#include <string.h> /* memcpy() */
#include <stdarg.h>
#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"
#include "misc.h" /* countargs(), getargs(), getstr(), file, infile, address */
#include "lineno.h"

int llbl;
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
		if (!prefix && *in == ':') {
			lp = ip;
			++in;
			skipsp(&in);
			prefix = getprefix(&in);
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
	label.address = addrl;
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
			flerrexit("duplicate label '%s'", mstrldup(li->name, idlen(li->name)));
	}
	arr_add(&labels, &label);
	ins.type = IT_LBL;
	ins.line = lineno_get();
	ins.d.lbl.lbl = labels.count - 1;
	arr_add(&inss, &ins);
}

int insfind(char *ip, char *argp) {
	int args[ARG_MAX];
	oc_t *oc = arch->ocs;
	ins_t ins;

	ins.line = lineno_get();
	if (cmpid(ip, "org")) {
		getargs(argp, args, 1, 1);
		ins.type = IT_ORG;
		ins.d.org.address = args[0];
		address = ins.d.org.address;
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "file")) {
		char *file;
		parseargs(argp, "s", &file);
		lineno_pushfile(file, 0, 0);
		ins.type = IT_CTX;
		ins.d.ctx = lineno_getctx();
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "expands")) {
		char *name, *file;
		unsigned int line;
		parseargs(argp, "isn", &name, &file, &line);
		name = mstrldup(name, idlen(name));
		lineno_pushmacro(name, file, line);
		ins.type = IT_CTX;
		ins.d.ctx = lineno_getctx();
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "endfile") || cmpid(ip, "endexp")) {
		if (argp != NULL)
			flerrexit("too many arguments");
		if (lineno.count == 1)
			flerrexit("context stack underrun");
		lineno_dropctx();
		ins.type = IT_CTX_END;
		arr_add(&inss, &ins);
	}
	if (cmpid(ip, "line")) {
		getargs(argp, args, 1, 1);
		lineno_set(args[0] - 1);
		return 1;
	}
	if (prefix)
		return 0;
	if (cmpid(ip, "data")) {
		int n = countargs(argp);
		ins.type = IT_DAT;
		ins.d.data.args = argp;
		address += n * arch->dlen / arch->align;
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
			address += ins.d.ins.len / arch->align;
			return 1;
		}
		++oc;
	}
	return 0;
}

void assemble(char *code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));
	lineno_init();
	lineno_pushfile(infile, 1, 0);

	{ /* first pass */
		int r;

		address = 0;
		llbl = -1;
		run = 0;
		while ((r = parseln(code))) {
			addrl = address; /* case there is a label we have the address at start of line */
			if (r == 2)
				flerrexit("syntax error");
			if (lp != NULL)
				addlabel(lp);
			if (ip != NULL) {
				if (insfind(ip, argp))
					run = 0;
				else if (lp || pspc)
					flerrexit("no such instruction or directive '%s'", mstrldup(ip, idlen(ip)));
			}
			if (run == 0 || prefix) {
				lineno_inc();
				code = nextln;
				if (prefix)
					run = 0;
			}
		}
		{
			ins_t ins;
			ins.line = lineno_get();
			ins.type = IT_END;
			arr_add(&inss, &ins);
		}
	}
	if (lineno.count != 1)
		errexit("context stack unbalanced");
	{ /* second pass */
		ins_t *ins = (ins_t *) inss.data;
		int i, c, args[ARG_MAX];

		address = 0;

		while (ins->type != IT_END) {
			lineno_set(ins->line);
			switch (ins->type) {
				case IT_INS:
					c = getargs(ins->d.ins.args, args, 0, ARG_MAX);
					arch->acmp(ins->d.ins.oc, ins->d.ins.atype, c, args);
					address += ins->d.ins.len / arch->align;
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
					address += arch->dlen / arch->align * c;
					break;
				case IT_CTX:
					lineno_pushctx(ins->d.ctx);
					break;
				case IT_CTX_END:
					lineno_dropctx();
					break;
				case IT_LBL:
					llbl = ins->d.lbl.lbl;
					break;
			}
			++ins;
		}
	}
	llbl = -1; /* important */
	lineno_end();
}
