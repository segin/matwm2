#include <stdlib.h> /* NULL */
#include "mem.h"
#include "str.h" /* skipsp(), ctype(), etc */
#include "misc.h" /* flerrexit(), getword(), readfile(), getstr(), clearfile(), file */
#include "ppc.h"
#include "io.h"

arr_t defines = { NULL, 0, 0, 0 };
int level, ignore; /* depth & state of if/ifdef/ifndef directives */
int str, esc;
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

void _ppsub(char *in, define_t *parent, int argc, char *argv[]) {
	char *s, *id;
	char *_argv[ARG_MAX];
	int _argc, i;
	define_t *def;

	esc = str = 0;
	while (!(ctype(*in) & (CT_NL | CT_NUL))) {
		s = in;
		while (!(ctype(*in) & (CT_NL | CT_NUL | CT_LET | CT_SEP))) {
			if (esc)
				esc = 0;
			else strcheck(*in);
			++in;
		}
		vstr_addl(&tmp, s, in - s);
		id = in;
		getid(&in);

		if (parent != NULL && argc) {
			if (parent->argc < argc)
				flerrexit("too many arguments for macro");
			for (i = 0; i < argc; ++i) {
				if (cmpid(id, parent->argv[i])) {
					_ppsub(argv[i], NULL, 0, NULL);
					break;
				}
			}
			if (i != argc)
				continue;
		}

		_argc = 0;
		if (*in == '(') {
			++in;
			while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ')') {
				s = in;
				while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ',' && *in != ')')
					++in;
				_argv[_argc] = strldup(s, in - s);
				if (_argv[_argc] == NULL)
					errexit("strldup() failure");
				if (*in == ',')
					++in;
				++_argc;
				if (_argc == ARG_MAX)
					flerrexit("too many arguments for macro");
			}
			if (*in != ')')
				flerrexit("missing ')'");
			++in;
		}
		if (!str) {
			def = ((define_t *) defines.data) + defines.count - 1;
			for (i = 0; i < defines.count; ++i, --def) /* go backwards for overloads */
				if (cmpid(def->name, id) && !def->active) {
					def->active = 1;
					_ppsub(def->val, def, _argc, _argv);
					def->active = 0;
					break;
				}
		}
		if (str || i == defines.count)
			vstr_addl(&tmp, id, in - id);
	}

	for (--argc; argc >= 0; --argc)
		free(argv[argc]);
}

char *ppsub(char *in) {
	tmp.len = 0;
	_ppsub(in, NULL, 0, NULL);
	return tmp.data;
}

void _preprocess(ioh_t *out, char *in);

int ppfind(ioh_t *out, char *lp, char *ip, char *argp) {
	char *s;
	int wp;
	int args[ARG_MAX];

	if (cmpid(ip, "if")) {
		if (argp == NULL)
			flerrexit("too few arguments for if directive");
		argp = ppsub(argp);
		if (getargs(&argp, args) != 1)
			flerrexit("too many arguments for if directive");
		++level;
		if (!ignore && !args[0])
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "ifdef")) {
		wp = getword(&argp, &s);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on ifdef directive");
		++level;
		if (!ignore && deffind(s) == NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "ifndef")) {
		wp = getword(&argp, &s);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on ifndef directive");
		++level;
		if (!ignore && deffind(s) != NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "endif")) {
		if (argp != NULL)
			flerrexit("syntax error on endif directive");
		if (!level)
			flerrexit("endif without prior if/ifdef/ifndef");
		if (level == ignore)
			ignore = 0;
		--level;
		return 1;
	}
	if (cmpid(ip, "else")) {
		if (argp != NULL)
			flerrexit("syntax error on else directive");
		if (!level)
			flerrexit("else without prior if/ifdef/ifndef");
		if (level == ignore || !ignore)
			ignore = (ignore ? 0 : level);
		return 1;
	}
	if (ignore)
		return 0;
	if (cmpid(ip, "define")) {
		define_t def;
		wp = getword(&argp, &def.name);
		if (def.name == NULL)
			flerrexit("syntax error on define directive");
		if  (!(wp & WP_TSPC) && *argp == '(') {
			def.argc = 0;
			++argp;
			while (1) {
				getword(&argp, &s);
				if (s == NULL && *argp != ')')
					flerrexit("syntax error in macro parameter list");
				def.argv[def.argc] = s;
				if (*argp == ')') {
					++argp;
					break;
				}
				++(def.argc);
				if (*argp != ',')
					flerrexit("syntax error in macro parameter list");
				++argp;
				if (def.argc == ARG_MAX)
					flerrexit("too many arguments for macro");
			}
			++(def.argc);
			if (!skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
				flerrexit("syntax error on define directive");
		} else {
			def.argc = 0;
			if (!(wp & WP_TSPC) && !(ctype(*argp) & (CT_NL | CT_NUL)))
				flerrexit("syntax error on define directive");
		}
		def.val = argp;
		def.active = 0;
		arr_add(&defines, &def);
		return 1;
	}
	if (cmpid(ip, "msg")) {
		if (!argp)
			flerrexit("too few arguments for msg directive");
		s = getstr(&argp, 1);
		skipsp(&argp);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on msg directive");
		flmsg(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "error")) {
		if (!argp)
			flerrexit("too few arguments for msg directive");
		s = getstr(&argp, 1);
		skipsp(&argp);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on msg directive");
		flerrexit(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "warn")) {
		if (!argp)
			flerrexit("too few arguments for msg directive");
		s = getstr(&argp, 1);
		skipsp(&argp);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on msg directive");
		flwarn(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "include")) {
		char *data, *ofile = file, *s = argp;
		int oline = line;
		if (!argp)
			flerrexit("too few arguments for msg directive");
		file = getstr(&argp, 0);
		if (file == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on include directive");
		s = getstr(&s, 1);
		data = readfile(s);
		free(s);
		if (data == NULL)
			flerrexit("failed to include file '%s'", file);
		mfprintf(out, "file \"%s\"\n", file);
		line = 1;
		_preprocess(out, data);
		free(data);
		free(file);
		file = ofile;
		line = oline;
		mfprintf(out, "file \"%s\"\nline %u\n", file, oline);
		return 1;
	}
	return 0;
}

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

void _preprocess(ioh_t *out, char *in) {
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
					if (ctype(*p) & CT_NL)
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

	{ /* fix escaped newlines and remove dos newlines */
		char *p, *c;
		int i, nl = 0;
		p = c = in;
		while (*c) {
			switch (*c) {
				case '\\':
					i = 1;
					while (c[i] == '\r')
						++i;
					if (c[1] == '\n')
						c += i + 1;
						++nl;
					break;
				case '\r':
					/* dos newlines will be added by mfwrite(), if the user wants them in output */
					++c;
					break;
				case '\n':
					if (nl)
						while (nl--)
							*(p++) = '\n';
			}
			*p = *c;
			++p, ++c;
		}
		*p = 0;
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
			if (!(wp & WP_TSPC) && !(ctype(*in) & (CT_NL | CT_NUL)))
				flerrexit("invalid character in local label"); /* can only happen with local label */
			lp = ip;
			wp = getword(&in, &ip);
		}
		if (ip == NULL) {
			if (pps && !(ctype(*in) & (CT_NL | CT_NUL)))
				flerrexit("invalid preprocessor directive");
			goto endln;
		}
		if (!(ctype(*in) & (CT_NL | CT_NUL))) {
			if (wp & WP_TSPC)
				argp = in;
			else if (pps) flerrexit("invalid preprocessor directive");
		}
		if ((r = ppfind(out, lp, ip, argp)))
			goto endln;
		if (lp == NULL && !(wp & WP_PSPC)) {
			lp = ip;
			wp = getword(&in, &ip);
			if (ip == NULL)
				goto endln;
			if (!(ctype(*in) & (CT_NL | CT_NUL))) {
				if (wp & WP_TSPC)
					argp = in;
				else goto endln;
			}
			r = ppfind(out, lp, ip, argp);
		}
		endln:
		while (!(ctype(*in) & (CT_NUL | CT_NL)))
			++in;
		skipnl(&in);
		if (!r && !ignore) {
			ppsub(lnstart);
			mfwrite(out, tmp.data, tmp.len);
		}
		mfprint(out, "\n");
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
void preprocess(ioh_t *out, char *in) {
	file = infile;
	vstr_new(&tmp);
	arr_new(&defines, sizeof(define_t));
	line = 1;
	level = ignore = 0;
	_preprocess(out, in);
	vstr_free(&tmp);
	arr_free(&defines);
}
