#include "mem.h"
#include "str.h" /* skipsp(), alfa[], etc */
#include "misc.h" /* getword(), linebuf */
#include "as.h" /* aerrexit(), initfile() */
#include "ppc.h"

arr_t defines = { NULL, 0, 0, 0 };
int level, ignore; /* depth & state of if/ifdef/ifndef directives */
string_t out;
string_t tmp;
string_t tmp2;

char *deffind(char *name) {
	define_t *def = ((define_t *) defines.data);
	char *ret = NULL;
	int i;

	for (i = 0; i < defines.count; ++i, ++def)
		if (cmpid(def->name, name))
			ret = def->arg;
	return ret;
}

void ppsub(char *in) {
	char *s, *d;
	int c;
	start:
	c = 0;
	tmp.len = 0;
	while (!(alfa[(unsigned char) *in] & (CT_NL | CT_NUL))) {
		s = in;
		while (!(alfa[(unsigned char) *in] & (CT_NL | CT_NUL | CT_LET | CT_SEP)))
			++in;
		vstr_addl(&tmp, s, in - s);
		s = in;
		if ((d = deffind(in)) != NULL) {
			vstr_addl(&tmp, d, idlen(d));
			getid(&in);
			++c;
		} else {
			getid(&in);
			vstr_addl(&tmp, s, in - s);
		}
	}
	if (c) {
		tmp2.len = 0;
		vstr_addl(&tmp2, tmp.data, tmp.len);
		in = tmp2.data;
		goto start;
	}
}

int ppfind(char *lp, char *ip, char *argp) {
	char *s;
	int wp;
	int args[ARG_MAX];

	if (cmpid(ip, "if")) {
		if (argp == NULL)
			aerrexit("too few arguments for if directive");
		ppsub(argp);
		if (getargs(&tmp.data, args) != 1)
			aerrexit("too many arguments for if directive");
		++level;
		if (!ignore && !args[0])
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "ifdef")) {
		wp = getword(&argp, &s);
		if (s == NULL || !(alfa[(unsigned char) *argp] & (CT_NL | CT_NUL)))
			aerrexit("syntax error on ifdef directive");
		++level;
		if (!ignore && deffind(s) == NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "ifndef")) {
		wp = getword(&argp, &s);
		if (s == NULL || !(alfa[(unsigned char) *argp] & (CT_NL | CT_NUL)))
			aerrexit("syntax error on ifndef directive");
		++level;
		if (!ignore && deffind(s) != NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "endif")) {
		if (argp != NULL)
			aerrexit("syntax error on endif directive");
		if (!level)
			aerrexit("endif without prior if/ifdef/ifndef");
		if (level == ignore)
			ignore = 0;
		--level;
		return 1;
	}
	if (cmpid(ip, "else")) {
		if (argp != NULL)
			aerrexit("syntax error on else directive");
		if (!level)
			aerrexit("else without prior if/ifdef/ifndef");
		if (level == ignore || !ignore)
			ignore = (ignore ? 0 : level);
		return 1;
	}
	if (ignore)
		return 0;
	if (cmpid(ip, "define")) {
		define_t def;
		wp = getword(&argp, &def.name);
		if (def.name == NULL || (!(wp & WP_TSPC) && !(alfa[(unsigned char) *argp] & (CT_NL | CT_NUL))))
			aerrexit("syntax error on define directive");
		def.arg = argp;
		arr_add(&defines, &def);
		return 1;
	}
	return 0;
}

int getprefix(char **src) {
	char *p = *src;
	int n = 0;
	skipsp(&p);
	while (alfa[(unsigned char) *p] & (CT_PPC))
		++p, ++n;
	if (n) {
		*src = p;
		skipsp(src);
	}
	return n;
}

int _preprocess(char *in, char **ret) {
	char *lnstart, *lp, *ip, *argp;
	int wp, r, pps;

	/* strip comments */
	{
		char *p = in;
		int iscomment = 0, nestcomment = 0, str = 0;

		while (*p) {
			if (!iscomment && !nestcomment && (p[0] != '\\' || p[0] != '\'') && p[1] == '"')
				str ^= 1;
			if (!str) {
				if (p[0] == '/' && p[1] == '*') {
					++nestcomment;
				} else if (nestcomment && p[0] == '*' && p[1] == '/') {
					--nestcomment;
					p[0] = p[1] = ' ';
					++p;
				}
				if (iscomment || nestcomment) {
					if (alfa[(unsigned char) *p] & CT_NL)
						iscomment = 0;
					else *p = ' ';
				} else if (*p == ';') {
					*p = ' ';
					iscomment = 1;
				}
			}
			++p;
		}
	}

	/* this very similar to assemble() */
	while (*in) {
		lnstart = in;
		lp = NULL;
		argp = NULL;
		r = 0;
		pps = getprefix(&in);
		wp = getword(&in, &ip);
		if (wp & (WP_LABEL | WP_LOCAL)) { /* we have label and we're sure about it */
			if (!(wp & WP_TSPC) && !(alfa[(unsigned char) *in] & (CT_NL | CT_NUL)))
				aerrexit("invalid character in local label"); /* can only happen with local label */
			lp = ip;
			wp = getword(&in, &ip);
		}
		if (ip == NULL) {
			if (pps && !(alfa[(unsigned char) *in] & (CT_NL | CT_NUL)))
				aerrexit("invalid preprocessor directive");
			goto endln;
		}
		if (!(alfa[(unsigned char) *in] & (CT_NL | CT_NUL))) {
			if (wp & WP_TSPC)
				argp = in;
			else if (pps) aerrexit("invalid preprocessor directive");
		}
		if ((r = ppfind(lp, ip, argp)))
			goto endln;
		if (lp == NULL && !(wp & WP_PSPC)) {
			lp = ip;
			wp = getword(&in, &ip);
			if (ip == NULL)
				goto endln;
			if (!(alfa[(unsigned char) *in] & (CT_NL | CT_NUL))) {
				if (wp & WP_TSPC)
					argp = in;
				else goto endln;
			}
			r = ppfind(lp, ip, argp);
		}
		endln:
		while (!(alfa[(unsigned char) *in] & (CT_NUL | CT_NL)))
			++in;
		skipnl(&in);
		if (!r && !ignore) {
			ppsub(lnstart);
			vstr_addl(&out, tmp.data, tmp.len);
		}
		vstr_add(&out, dosnl ? "\r\n" : "\n");
		++line;
	}

	*ret = out.data;
	return out.len;
}

/* preprocess(in, ret)
 *
 * description
 *   strips comments from in (on the string itself replaces them with space)
 *   evaluates preprocessor directives and builds with them preprocessed data
 *   on a new string, which *ret will point to
 *
 * arguments
 *   char * in: zero terminated string to be preprocessed
 *   char ** ret: *ret is set to pointer with preprocessed data
 *
 * return value
 *   int
 *   length of *ret
 */
int preprocess(char *in, char **ret) {
	int n;
	initfile();
	vstr_new(&out);
	vstr_new(&tmp);
	vstr_new(&tmp2);
	arr_new(&defines, sizeof(define_t));
	line = 1;
	level = 0;
	ignore = 0;
	n = _preprocess(in, ret);
	vstr_free(&tmp);
	vstr_free(&tmp2);
	return n;
}

