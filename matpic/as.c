/*************
 * assembler *
 *************/

#include <stdlib.h> /* NULL */
#include <string.h> /* memcpy() */
#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"
#include "misc.h" /* getargs(), getstr(), file, infile, address */
#include "lineno.h"

int llbl;
unsigned int addrl;

arr_t inss = { NULL, 0, 0, 0 }; /* these need to be 0 so cleanup() before assemble won't fail */
arr_t labels = { NULL, 0, 0, 0 };
string_t outbuf = { NULL, 0, 0 };

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
	ins.lbl.lbl = labels.count - 1;
	arr_add(&inss, &ins);
}

int countargs(char *src) {
	int n = 1;
	if (src == NULL)
		return 0;
	while(!(ctype(*src) & (CT_NUL | CT_NL))) {
		switch (*src) {
			case ',':
				++n;
				break;
/*			case '"':
			case '\'':
				break; */
		}
		++src;
	}
	return n;
}

void adddata(int size, char *argp) {
	ins_t ins;
	int n = countargs(argp);
	if (n == 0)
		return;
	ins.type = IT_DAT;
	ins.data.args = argp;
	ins.data.size = size;
	ins.data.len = n;
	ins.data.pad = 0;
	if (n * size % arch->align)
		ins.data.pad = arch->align - n * size % arch->align;
	address += (n * size + ins.data.pad) / arch->align;
	arr_add(&inss, &ins);
	vstr_skip(&outbuf, n);
}

int insfind(char *ip, char *argp) {
	sll args[ARG_MAX];
	oc_t *oc = arch->ocs;
	ins_t ins;

	ins.line = lineno_get();
	if (cmpid(ip, "org")) {
		getargs(argp, args, 1, 1);
		ins.type = IT_ORG;
		ins.org.address = args[0];
		address = ins.org.address;
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "file")) {
		char *file;
		parseargs(argp, "s", &file);
		lineno_pushfile(file, 0, 0);
		ins.type = IT_CTX;
		ins.ctx.ctx = lineno_getctx();
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
		ins.ctx.ctx = lineno_getctx();
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
	if (cmpid(ip, "db")) {
		adddata(1, argp);
		return 1;
	}
	if (cmpid(ip, "dw")) {
		adddata(2, argp);
		return 1;
	}
	if (cmpid(ip, "dd")) {
		adddata(4, argp);
		return 1;
	}
	if (cmpid(ip, "data")) {
		adddata(arch->align, argp);
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
			ins.ins.oc = oc;
			ins.ins.args = argp;
			arr_add(&inss, &ins);
			vstr_skip(&outbuf, oc->len);
			address += oc->len / arch->align;
			return 1;
		}
		++oc;
	}
	return 0;
}

void assemble(char *code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));
	vstr_new(&outbuf);
	lineno_init();
	lineno_pushfile(infile, 1, 0);

	{ /* first pass */
		int r;

		llbl = -1;
		run = 0;
		address = 0;
		{ /* start with an org, to be friendly for output modules */
			ins_t ins;
			ins.type = IT_ORG;
			ins.org.address = address;
			arr_add(&inss, &ins);
		}
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
		int i, j, c;
		sll args[ARG_MAX];
		unsigned char *bufp = (unsigned char *) outbuf.data;
		unsigned char op[8];
		char **lorgend = NULL;

		address = 0;
		while (ins->type != IT_END) {
			lineno_set(ins->line);
			switch (ins->type) {
				case IT_INS:
					c = getargs(ins->ins.args, args, 0, ARG_MAX);
					memcpy(op, ins->ins.oc->oc, ins->ins.oc->len);
					arch->acmp(op, ins->ins.oc->atype, c, args);
					for (i = 0; i < ins->ins.oc->len; ++i)
						bufp[i] = op[i - i % arch->align + arch->align - 1 - arch->ord[i % arch->align]];
					address += ins->ins.oc->len / arch->align;
					bufp += ins->ins.oc->len;
					break;
				case IT_DAT:
					getargs(ins->data.args, args, 0, ARG_MAX);
					mfprintf(mstderr, "%d %d %d\n", ins->data.size, ins->data.len, ins->data.pad);
					memset(bufp, 0, ins->data.len * ins->data.size + ins->data.pad);
					for (i = 0; i < ins->data.len; ++i) {
						for (j = 0; j < ins->data.size; ++j)
							op[j] = (args[i] & (0xFF << (j * 8))) >> (j * 8);
						for (j = 0; j < ins->data.size; ++j) {
							c = i * ins->data.size + j;
							bufp[c - (c % arch->align) + arch->ord[c % arch->align]] = op[j];
						}
					}
					bufp += ins->data.len * ins->data.size + ins->data.pad;
					address += (ins->data.len * ins->data.size + ins->data.pad) / arch->align;
					break;
				case IT_ORG:
					if (lorgend != NULL)
						*lorgend = (char *) bufp;
					lorgend = &ins->org.end;
					address = ins->org.address;
					break;
				case IT_CTX:
					lineno_pushctx(ins->ctx.ctx);
					break;
				case IT_CTX_END:
					lineno_dropctx();
					break;
				case IT_LBL:
					llbl = ins->lbl.lbl;
					break;
			}
			++ins;
		}
		if (lorgend != NULL)
			*lorgend = (char *) bufp;
	}
	llbl = -1; /* important */
	lineno_end();
}
