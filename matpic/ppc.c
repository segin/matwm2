#include "mem.h"
#include "str.h" /* skipsp(), alfa[], etc */
#include "misc.h" /* getword(), linebuf */
#include "as.h" /* aerrexit(), initfile(), setfile() */
#include "host.h" /* readfile(), strcpy() */
#include "ppc.h"

arr_t defines = { NULL, 0, 0, 0 };
int level, ignore; /* depth & state of if/ifdef/ifndef directives */
int str, esc;
string_t out;
string_t tmp;

void strcheck(char c) {
	switch (c) {
		case '\\':
			if (str)
				++esc;
			return;
		case '"':
			if (!(str && esc) || str & 2)
				str ^= 1;
			break;
		case '\'':
			str ^= 2;
			break;
	}
	esc = 0;
}

char *deffind(char *name) {
	define_t *def = ((define_t *) defines.data);
	char *ret = NULL;
	int i;

	for (i = 0; i < defines.count; ++i, ++def)
		if (cmpid(def->name, name))
			ret = def->val; /* don't break, so we can overload macros */
	return ret;
}

void _ppsub(char *in);

int __ppsub(char *name) {
	define_t *def = ((define_t *) defines.data) + defines.count - 1;
	int i;

	for (i = 0; i < defines.count; ++i, --def) /* go backwards for overloads */
		if (cmpid(def->name, name) && !def->active) {
			def->active = 1;
			_ppsub(def->val);
			def->active = 0;
			break;
		}
	return i != defines.count;
}

void _ppsub(char *in) {
	char *s;
	int c;
	esc = str = 0;
	while (!(alfa[(unsigned char) *in] & (CT_NL | CT_NUL))) {
		s = in;
		while (!(alfa[(unsigned char) *in] & (CT_NL | CT_NUL | CT_LET | CT_SEP))) {
			if (esc)
				esc = 0;
			else strcheck(*in);
			++in;
		}
		vstr_addl(&tmp, s, in - s);
		s = in;
		getid(&in);
		if (!str && __ppsub(s))
			++c;
		else vstr_addl(&tmp, s, in - s);
	}
}

char *ppsub(char *in) {
	tmp.len = 0;
	_ppsub(in);
	return tmp.data;
}

void _preprocess(char *in);

int ppfind(char *lp, char *ip, char *argp) {
	char *s;
	int wp;
	int args[ARG_MAX];

	if (cmpid(ip, "if")) {
		if (argp == NULL)
			aerrexit("too few arguments for if directive");
		argp = ppsub(argp);
		if (getargs(&argp, args) != 1)
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
		def.val = argp;
		def.active = 0;
		arr_add(&defines, &def);
		return 1;
	}
	if (cmpid(ip, "msg")) {
		char *msg;
		if (!argp)
			aerrexit("too few arguments for msg directive");
		msg = getstr(&argp);
		skipsp(&argp);
		if (msg == NULL || !(alfa[(unsigned char) *argp] & (CT_NL | CT_NUL)))
			aerrexit("syntax error on msg directive");
		flmsg(file, line, msg);
		free(msg);
		return 1;
	}
	if (cmpid(ip, "error")) {
		char *msg;
		if (!argp)
			aerrexit("too few arguments for msg directive");
		msg = getstr(&argp);
		skipsp(&argp);
		if (msg == NULL || !(alfa[(unsigned char) *argp] & (CT_NL | CT_NUL)))
			aerrexit("syntax error on msg directive");
		flerrexit(file, line, msg);
		free(msg);
		return 1;
	}
	if (cmpid(ip, "warn")) {
		char *msg;
		if (!argp)
			aerrexit("too few arguments for msg directive");
		msg = getstr(&argp);
		skipsp(&argp);
		if (msg == NULL || !(alfa[(unsigned char) *argp] & (CT_NL | CT_NUL)))
			aerrexit("syntax error on msg directive");
		flwarn(file, line, msg);
		free(msg);
		return 1;
	}
	if (cmpid(ip, "include")) {
		char *fn, *data, ofile[FN_MAX];
		if (!argp)
			aerrexit("too few arguments for msg directive");
		fn = getstr(&argp);
		skipsp(&argp);
		if (fn == NULL || !(alfa[(unsigned char) *argp] & (CT_NL | CT_NUL)))
			aerrexit("syntax error on include directive");
		data = readfile(fn);
		if (data == NULL)
			aerrexit("failed to include file");
		strcpy(ofile, file);
		strcpy(file, fn);
		vstr_add(&out, "file \"");
		vstr_add(&out, fn);
		vstr_add(&out, "\"\n");
		free(fn);
		_preprocess(data);
		free(data);
		strcpy(file, ofile);
		vstr_add(&out, "file \"");
		vstr_add(&out, file);
		vstr_add(&out, "\"\n");
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

void _preprocess(char *in) {
	char *lnstart, *lp, *ip, *argp;
	int wp, r, pps;

	/* strip comments */
	{
		char *p = in;
		int iscomment = 0, nestcomment = 0, str = 0;

		esc = str = 0;
		while (*p) {
			if (!iscomment && !nestcomment)
				strcheck(*p);
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
		vstr_add(&out, "\n");
		++line;
	}
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
	initfile();
	vstr_new(&out);
	vstr_new(&tmp);
	arr_new(&defines, sizeof(define_t));
	line = 1;
	level = ignore = 0;
	_preprocess(in);
	vstr_free(&tmp);
	arr_free(&defines);
	*ret = out.data;
	return out.len;
}
