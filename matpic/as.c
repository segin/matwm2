/*************
 * assembler *
 *************/

#include <stdlib.h> /* NULL */
#include <string.h> /* memcpy(), strlen(), memset() */
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
arr_t map = { NULL, 0, 0, 0 };
arr_t strargs = { NULL, 0, 0, 0 };

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
	ins.head.type = IT_LBL;
	ins.head.line = lineno_getreal();
	ins.lbl.lbl = labels.count - 1;
	arr_add(&inss, &ins);
}

int addstrdata(char **src, int size) {
	if (**src != '"' && **src != '\'')
		return -1;
	{
		int n;
		char *s, *b = *src;
		strarg_t arg;

		s = getstr(src);
		skipsp(src);
		if (**src != ',' && !(ctype(**src) & (CT_NUL | CT_NL))) {
			free(s);
			--*src;
			return -1;
		}
		n = strlen(s);
		arg.str = s;
		arr_add(&strargs, &arg);
		if (n % size)
			n += size - n % size;
		vstr_fill(&outbuf, n, 0);
		return n;
	}
}

void adddata(int size, char *src) {
	int n = 0, s;
	char *argp = src;
	if (src == NULL)
		return;
	goto start;
	while(!(ctype(*src) & (CT_NUL | CT_NL))) {
		if (*src == ',') {
			++src;
			start:
			skipsp(&src);
			if ((s = addstrdata(&src, size)) == -1) {
				vstr_fill(&outbuf, size, 1);
				++n;
			} else n += s;
		} else ++src;
	}
	if (n) {
		ins_t ins;
		int pad;
		ins.head.line = lineno_getreal();
		ins.head.type = IT_DAT;
		ins.data.args = argp;
		ins.data.size = size;
		pad = (n * size % arch->align) ? pad = arch->align - n * size % arch->align : 0;
		address += (n * size + pad) / arch->align;
		arr_add(&inss, &ins);
	}
}

int insfind(char *ip, char *argp) {
	sll args[ARG_MAX];
	oc_t *oc = arch->ocs;
	ins_t ins;

	ins.head.line = lineno_getreal();
	if (cmpid(ip, "org")) {
		getargs(argp, args, 1, 1);
		ins.head.type = IT_ORG;
		address = args[0];
		ins.org.address = address;
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "file")) {
		char *file;
		parseargs(argp, "s", &file);
		lineno_pushfile(file, 0, 0);
		ins.head.type = IT_CTX;
		ins.ctx.ctx = lineno_getctx();
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "expands")) {
		char *name, *file;
		sll line;
		parseargs(argp, "isn", &name, &file, &line);
		name = mstrldup(name, idlen(name));
		lineno_pushmacro(name, file, line);
		ins.head.type = IT_CTX;
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
		ins.head.type = IT_CTX_END;
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
	if (cmpid(ip, "radix")) {
		setradix(argp);
		return 1;
	}

	while (oc->name != NULL) {
		if (cmpid(ip, oc->name)) {
			ins.head.type = IT_INS;
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

void colcheck(unsigned long addr, unsigned long end) {
	int i;
	for (i = 0; i < map.count; ++i) {
		if (arr_item(map, map_t, i)->start != arr_item(map, map_t, i)->end &&
			((arr_item(map, map_t, i)->start <= addr && arr_item(map, map_t, i)->end > addr) || /* check if map item encompasses what we checkin */
		    (arr_item(map, map_t, i)->start <= addr && arr_item(map, map_t, i)->end > end) ||
			(arr_item(map, map_t, i)->start >= addr && arr_item(map, map_t, i)->end < addr) || /* check the opposite also */
		    (arr_item(map, map_t, i)->start >= addr && arr_item(map, map_t, i)->end < end))) {
			flerrexit("adress collision");
		}
	}
}

void assemble(char *code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));
	arr_new(&map, sizeof(map_t));
	arr_new(&strargs, sizeof(strarg_t));
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
			ins.head.line = lineno_getreal();
			ins.head.type = IT_ORG;
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
			ins.head.line = lineno_getreal();
			ins.head.type = IT_END;
			arr_add(&inss, &ins);
		}
	}
	if (lineno.count != 1)
		errexit("context stack unbalanced");
	{ /* second pass */
		ins_t *ins = (ins_t *) inss.data;
		sll args[ARG_MAX], arg;
		unsigned char *bufp = (unsigned char *) outbuf.data; /* this works coz we do not realloc anymore */
		char **lorgend = NULL; /* this too */
		unsigned char op[8];
		map_t mapitem = { 0, 0 };
		char *argp;
		int i, j, a;
		ull n;
		strarg_t *strarg = (strarg_t *) strargs.data;

		address = 0;
		while (ins->head.type != IT_END) {
			lineno_set(ins->head.line);
			switch (ins->head.type) {
				case IT_INS:
					a = getargs(ins->ins.args, args, 0, ARG_MAX);
					memcpy(op, ins->ins.oc->oc, ins->ins.oc->len);
					arch->acmp(op, ins->ins.oc->atype, a, args);
					for (i = 0; i < ins->ins.oc->len; ++i)
						bufp[i] = op[i - i % arch->align + arch->ord[i % arch->align]];
					bufp += ins->ins.oc->len;
					address += ins->ins.oc->len / arch->align;
					colcheck(mapitem.end, address);
					mapitem.end = address;
					break;
				case IT_DAT:
					a = 0;
					argp = ins->data.args;
					while (1) {
						if (bufp[a] == 0) { /* string argument */
							i = strlen(strarg->str);
							memcpy(bufp + a, strarg->str, i);
							a += i;
							free(strarg->str);
							++strarg;
							while (!(ctype(*argp) & (CT_NUL | CT_NL)) && *argp != ',')
								++argp;
							if (a % ins->data.size)
								a += ins->data.size - a % ins->data.size;
							if (ctype(*argp) & (CT_NUL | CT_NL))
								break;
							++argp;
						} else {
							arg = numarg(&argp);
							if (ctype(*argp) & (CT_NUL | CT_NL) || *argp == ',') {
								n = ntt(arg);
								for (i = 0; i < ins->data.size; ++i)
									op[ins->data.size - 1 - i] = (arg & (0xFF << (i * 8))) >> (i * 8);
								memcpy(bufp + a, op, ins->data.size);
								a += ins->data.size;
								if (*argp != ',')
									break;
							}
							if (*argp != ',')
								flerrexit("your argument is invalid");
							++argp;
							skipsp(&argp);
							if (ctype(*argp) & (CT_NUL | CT_NL))
								flerrexit("expression expected");
						}
					}
					if (a % arch->align) {
						memset(bufp + a, 0, arch->align - a % arch->align);
						a += arch->align - a % arch->align;
					}
					for (i = 0; i < a; i += arch->align) {
						memcpy(op, bufp + i, arch->align);
						for (j = 0; j < arch->align; ++j) {
							bufp[i + arch->ord[j]] = op[j] & arch->mask[j];
							if (op[j] & ~arch->mask[j])
								flwarn("data out of range, truncated");
						}
					}
					bufp += a;
					address += a / arch->align;
					colcheck(mapitem.end, address);
					mapitem.end = address;
				case IT_ORG:
					if (lorgend != NULL)
						*lorgend = (char *) bufp;
					lorgend = &ins->org.end;
					address = ins->org.address;
					arr_add(&map, &mapitem);
					mapitem.start = mapitem.end = address;
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
	arr_free(&map);
	arr_free(&strargs);
}
