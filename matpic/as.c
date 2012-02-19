/*************
 * assembler *
 *************/

#include "host.h" /* memcpy() */
#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"
#include "misc.h" /* flerrexit(), getargs(), infile, address, line */

char file[FN_MAX];
label_t *cnl = NULL;

arr_t inss = { NULL, 0, 0, 0 };
arr_t labels = { NULL, 0, 0, 0 };

void aerrexit(char *msg) {
	flerrexit(file, line, msg);
}

int countargs(char *src) {
	int n = 1;
	if (src == NULL)
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

int insfind(char *lp, char *ip, char *argp) {
	oc_t *oc = arch->ocs;
	ins_t ins;
	int args[ARG_MAX];

	ins.line = line;
	if (cmpid(ip, "org")) {
		if (getargs(&argp, args) != 1)
			aerrexit("invalid number of arguments to org directive");
		ins.type = IT_ORG;
		ins.d.org.address = args[0] * arch->align;
		address = ins.d.org.address;
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "file")) {
		if (argp == NULL)
			aerrexit("'file' directive needs an argument");
		setfile(argp);
		ins.type = IT_FIL;
		ins.d.file.file = argp;
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "line")) {
		if (getargs(&argp, args) != 1)
			aerrexit("'line' directive wants exactly 1 argument");
		line = args[0] - 1;
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
		label_t label, *li;
		ins_t ins;
		char *lp, *ip, *argp;
		int wp, i;
		unsigned int addrl;

		initfile();
		line = 1;
		address = 0;
		while (*code) {
			ins.line = line;
			addrl = address; /* case there is a label we have the address at start of line */
			lp = NULL;
			argp = NULL;
			wp = getword(&code, &ip);
			if (wp & (WP_LABEL | WP_LOCAL)) { /* we have label and we're sure about it */
				if (!(wp & WP_TSPC) && !(alfa[(unsigned char) *code] & (CT_NL | CT_NUL)))
					aerrexit("invalid character in local label"); /* can only happen with local label */
				lp = ip;
				wp = getword(&code, &ip);
			}
			if (ip == NULL) {
				if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL)))
					aerrexit("invalid identifier");
				goto endln;
			}
			if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL))) {
				if (wp & WP_TSPC)
					argp = code;
				else aerrexit("invalid identifier");
			}
			if (insfind(lp, ip, argp))
				goto endln;
			if (lp == NULL) {
				if (!(wp & WP_PSPC)) {
					lp = ip;
					wp = getword(&code, &ip);
					if (ip == NULL) {
						if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL)))
							aerrexit("invalid identifier");
						goto endln;
					}
					if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL))) {
						if (wp & WP_TSPC)
							argp = code;
						else aerrexit("invalid identifier");
					}
					if (!insfind(lp, ip, argp))
						aerrexit("no such instruction/directive");
				} else aerrexit("no such instruction/directive");
			}
			endln:
			if (lp != NULL) { /* we has a lebel */
				label.name = lp;
				label.address = addrl / arch->align;
				for (i = 0; i < labels.count; ++i) { /* check if already exists */
					li = (label_t *) ((label_t *) labels.data) + i;
					if (cmpid(li->name, lp))
						aerrexit("duplicate label");
				}
				arr_add(&labels, &label);
				if (*lp != '.') {
					ins.type = IT_LAB;
					ins.d.lab.ptr = (label_t *) labels.data + labels.count - 1;
					arr_add(&inss, &ins);
				}
			}
			while (!(alfa[(unsigned char) *code] & (CT_NUL | CT_NL)))
				++code;
			skipnl(&code);
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
				case IT_LAB:
					cnl = ins->d.lab.ptr;
					break;
			}
			++ins;
		}
		cnl = NULL; /* so getval() is not confused when used from other functions */
	}
}

