#include <stdlib.h> /* NULL */
#include "mem.h"
#include "str.h" /* skipsp(), ctype(), etc */
#include "misc.h" /* flerrexit(), getword(), readfile(), getstr(), clearfile(), file */
#include "ppc.h"
#include "io.h"

arr_t defines = { NULL, 0, 0, 0 };
arr_t macros = { NULL, 0, 0, 0 };

arglist_t *defargs;
arglist_t *macargs;

int level, ignore; /* depth & state of if/ifdef/ifndef directives */
int str, esc, macro, explvl, rep0, rep, repno;
char *repstart;

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
			if (!(str && esc) || str & 1)
				str ^= 2;
			break;
		case '\n':
			str = 0;
			break;
	}
	esc = 0;
}

define_t *deffind(char *name) {
	define_t *def = ((define_t *) defines.data);
	int i;
	for (i = 0; i < defines.count; ++i, ++def)
		if (cmpid(def->name, name))
			return def;
	return NULL;
}

macro_t *macrofind(char *name) {
	macro_t *mac = ((macro_t *) macros.data);
	int i;
	for (i = 0; i < macros.count; ++i, ++mac)
		if (cmpid(mac->name, name))
			return mac;
	return NULL;
}

char *sppsub(char *in, macro_t *mac, char end);

void _ppsub(ioh_t *out, char *in, macro_t *mac, define_t *parent, char end) {
	char *s, *id;
	int i;
	define_t *def;
	arglist_t args, *argt;

	macro = esc = str = 0;
	while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != end) {
		s = in;
		while (!(ctype(*in) & (CT_NL | CT_NUL | CT_LET | CT_SEP)) && *in != '[' && *in != '@' && *in != end) {
			if (esc)
				esc = 0;
			else strcheck(*in);
			++in;
		}
		mfwrite(out, s, in - s);
		if (*in == end || ctype(*in) & (CT_NL | CT_NUL))
			return;
		if (!str && *in == '@') {
			++in;
			if (*in == '@') {
				++in;
				mfprintf(out, "%ut", repno);
				continue;
			}
			if (*in == '<') {
				char *p = in, *t = in;
				while (!(ctype(*t) & (CT_NL | CT_NUL)) && *t != '>')
					++t;
				if (*t != '>')
					flerrexit("missing '>'");
				++t;
				in = t;
				p = t = sppsub(p + 1, mac, '>');
				i = numarg(&t);
				if (*t)
					flerrexit("exess data in <> group");
				free(p);
			} else i = getval(&in);
			if (macargs == NULL) {
				mfprint(out, "0t");
				continue;
			}
			if (i > macargs->argc - mac->argc)
				flerrexit("macro wants nonexistant argument @%i", i);
			if (i == 0)
				mfprintf(out, "%ut", macargs->argc - mac->argc);
			else mfprint(out, macargs->argv[mac->argc + i - 1]);
			continue;
		}
		if (!str && *in == '[') {
			char *p;
			++in;
			p = id = sppsub(in, mac, ']');
			mfprintf(out, "%xh", numarg(&p));
			if (*p)
				flerrexit("exess data in preprocessor evaluation group");
			free(id);
			while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ']')
				++in;
			if (*in != ']')
				flerrexit("expected ']' after expression");
			++in;
			continue;
		}
		id = in;
		getid(&in);
		if (!str) {
			if (macargs != NULL && macargs->argc) {
				for (i = 0; i < mac->argc; ++i) {
					if (cmpid(id, mac->argv[i])) {
						_ppsub(out, macargs->argv[i], NULL, NULL, 0);
						break;
					}
				}
				if (i != mac->argc)
					continue;
			}

			if (parent != NULL && defargs->argc) {
				for (i = 0; i < defargs->argc; ++i) {
					if (cmpid(id, parent->argv[i])) {
						_ppsub(out, defargs->argv[i], NULL, NULL, 0);
						break;
					}
				}
				if (i != defargs->argc)
					continue;
			}

			if ((def = deffind(id)) != NULL && !def->active) {
				args.argc = 0;
				if (*in == '(') {
					++in;
					while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ')') {
						s = in;
						if (args.argc + 1 < def->argc)
							while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ',' && *in != ')')
								++in;
						else while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ')')
								++in;
						args.argv[args.argc] = strldup(s, in - s);
						if (args.argv[args.argc] == NULL)
							errexit("strldup() failure");
						if (*in == ',')
							++in;
						++args.argc;
						if (args.argc == ARG_MAX)
							flerrexit("too many arguments for macro");
					}
					if (*in != ')')
						flerrexit("missing ')'");
					++in;
				}
				if (args.argc < def->argc)
					flerrexit("too few arguments for macro");

				argt = defargs;
				defargs = &args;
				++def->active;
				_ppsub(out, def->val, mac, def, 0);
				--def->active;
				defargs = argt;
				for (--args.argc; args.argc >= 0; --args.argc)
					free(args.argv[args.argc]);
				continue;
			}
		}
		mfwrite(out, id, in - id);
	}
}

void ppsub(ioh_t *out, char *in, macro_t *mac, char end) {
	_ppsub(out, in, mac, NULL, end);
}

char *sppsub(char *in, macro_t *mac, char end) {
	char *ret;
	ioh_t *h = mmemopen(0);
	if (h == NULL)
		errexit("mmemopen() failed");
	ppsub(h, in, mac, end);
	mfwrite(h, "\0", 1);
	ret = mmemget(h);
	if (ret == NULL)
		errexit("out of memory");
	mfclose(h);
	return ret;
}

void define(char *argp, macro_t *mac, int eval) {
	define_t def, *p;
	char *s;
	int wp;
	if (argp == NULL)
		flerrexit("too few arguments for define directive");
	if (*argp == '<') {
		++argp;
		def.nptr = s = sppsub(argp, mac, '>');
		while (!(ctype(*argp) & (CT_NL | CT_NUL)) && *argp != '>')
			++argp;
		if (*argp != '>')
			flerrexit("missing '>'");
		++argp;
		getword(&s, &def.name);
		if (*s)
			flerrexit("syntax error on define directive");
		wp = WP_TSPC;
	} else {
		wp = getword(&argp, &def.name);
		def.nptr = NULL;
	}
	if (def.name == NULL)
		flerrexit("syntax error on define directive");
	def.argc = 0;
	if  (!(wp & WP_TSPC) && *argp == '(') {
		++argp;
		while (1) {
			getword(&argp, &s);
			if (s == NULL)
				flerrexit("syntax error in macro parameter list");
			if (s != NULL) {
				def.argv[def.argc] = s;
				++def.argc;
			}
			if (*argp == ')') {
				++argp;
				break;
			}
			if (*argp != ',')
				flerrexit("syntax error in macro parameter list");
			++argp;
			if (def.argc == ARG_MAX)
				flerrexit("too many arguments for macro");
		}
		if (!skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on define directive");
	} else if (!(wp & WP_TSPC) && !(ctype(*argp) & (CT_NL | CT_NUL)))
		flerrexit("syntax error on define directive");
	if (eval) {
		def.val = sppsub(argp, mac, 0);
		def.free = 1;
	} else {
		def.val = argp;
		def.free = 0;
	}
	def.active = 0;
	if ((p = deffind(def.name)) != NULL) {
		if (p->free)
			free(p->val);
		free(p->nptr);
		memcpy(p, &def, sizeof(define_t));
	}
	else arr_add(&defines, &def);
}

void _preprocess(ioh_t *out, char *in, macro_t *mac);

int ppfind(ioh_t *out, char *ip, char *argp, char **next, macro_t *mac) {
	char *s;
	int wp;

	if (!ignore && cmpid(ip, "endm")) {
		if (macro) {
			--macro;
			return 1;
		}
		if (explvl > 0) {
			--explvl;
			return 2;
		}
		flerrexit("endm without prior macro directive");
	}
	if (cmpid(ip, "macro")) {
		macro_t mac, *p;
		if (macro || rep0 || ignore) {
			++macro;
			return 1;
		}
		if (argp == NULL)
			flerrexit("macro directive needs more arguments");
		wp = getword(&argp, &mac.name);
		if (mac.name == NULL)
			flerrexit("syntax error on macro directive");
		mac.active = 0;
		mac.argc = 0;
		if (wp & WP_TSPC && !(ctype(*argp) & (CT_NL | CT_NUL)))
			while (1) {
				getword(&argp, &s);
				if (s == NULL)
					flerrexit("syntax error in macro parameter list");
				mac.argv[mac.argc] = s;
				++mac.argc;
				if (ctype(*argp) & (CT_NL | CT_NUL))
					break;
				if (*argp != ',')
					flerrexit("syntax error in macro parameter list");
				++argp;
				if (mac.argc == ARG_MAX)
					flerrexit("too many arguments for macro");
			}
		if (!(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on macro directive");
		mac.val = *next;
		if ((p = macrofind(mac.name)) != NULL)
			memcpy(p, &mac, sizeof(macro_t));
		else arr_add(&macros, &mac);
		++macro;
		return 1;
	}
	if (cmpid(ip, "rep")) {
		int args[ARG_MAX], rline = line;
		if (rep0 || ignore || macro) {
			++rep0;
			return 1;
		}
		if (argp == NULL)
			flerrexit("too few arguments for rep directive");
		s = argp = sppsub(argp, mac, 0);
		if (getargs(&argp, args) != 1)
			flerrexit("too many arguments for rep directive");
		if (!args[0])
			++rep0;
		else {
			int orepno = repno;
			++rep;
			for (repno = 0; repno < args[0]; ++repno) {
				if (!explvl)
					mfprintf(out, " line %ut\n", rline + 1);
				_preprocess(out, *next, mac);
			}
			repno = orepno;
			--rep;
			if (!explvl)
				mfprintf(out, " line %ut\n", rline);
		}
		++rep0;
		free(s);
		return 1;
	}
	if (cmpid(ip, "endrep")) {
		if (rep0) {
			--rep0;
			return 1;
		} else if (rep && !macro && !ignore)
			return 2;
		flerrexit("endrep without prior rep");
	}
	if (cmpid(ip, "if")) {
		int args[ARG_MAX];
		if (argp == NULL)
			flerrexit("too few arguments for if directive");
		s = argp = sppsub(argp, mac, 0);
		if (getargs(&argp, args) != 1)
			flerrexit("too many arguments for if directive");
		++level;
		if (!ignore && !macro && !rep0 && !args[0])
			ignore = level;
		free(s);
		return 1;
	}
	if (cmpid(ip, "ifdef")) {
		wp = getword(&argp, &s);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on ifdef directive");
		++level;
		if (!ignore && !macro && !rep0 && deffind(s) == NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "ifndef")) {
		wp = getword(&argp, &s);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on ifndef directive");
		++level;
		if (!ignore && !macro && !rep0 && deffind(s) != NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "endif")) {
		if (argp != NULL)
			flerrexit("syntax error on endif directive");
		if (!level)
			flerrexit("endif without prior if/ifdef/ifndef");
		if (level == ignore && !macro && !rep0)
			ignore = 0;
		--level;
		return 1;
	}
	if (cmpid(ip, "else")) {
		if (argp != NULL)
			flerrexit("syntax error on else directive");
		if (!level)
			flerrexit("else without prior if/ifdef/ifndef");
		if (!macro || !rep0)
			return 1;
		if (level == ignore || !ignore)
			ignore = (ignore ? 0 : level);
		return 1;
	}
	if (macro || ignore || rep0)
		return 0;
	if (cmpid(ip, "xdefine")) {
		define(argp, mac, 1);
		return 1;
	}
	if (cmpid(ip, "define")) {
		define(argp, mac, 0);
		return 1;
	}
	if (cmpid(ip, "undef")) {
		define_t *def;
		if (argp == NULL)
			flerrexit("too few arguments to undef");
		getword(&argp, &s);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error at undef directive");
		def = deffind(s);
		if (def != NULL) {
			if (def->free)
				free(def->val);
			--defines.count;
			memcpy(def, ((define_t *) defines.data + defines.count), sizeof(define_t));
		}
		return 1;
	}
	if (cmpid(ip, "msg")) {
		if (argp == NULL)
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
		if (argp == NULL)
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
		if (argp == NULL)
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
		char *data, *ofile = file, *s, *t;
		int oline = line;
		if (argp == NULL)
			flerrexit("too few arguments for msg directive");
		t = argp;
		s = getstr(&t, 0);
		if (s == NULL || !(ctype(*t) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on include directive");
		t = getstr(&argp, 1);
		data = readfile(t);
		free(t);
		if (data == NULL)
			flerrexit("failed to include file '%s'", s);
		file = s;
		mfprintf(out, " file \"%s\"\n", file);
		line = 1;
		_preprocess(out, data, mac);
		free(data);
		free(file);
		file = ofile;
		line = oline;
		mfprintf(out, " file \"%s\"\n", file);
		if (!explvl)
			mfprintf(out, " line %ut\n", oline + 1);
		else mfprintf(out, " line %ut\n nocount\n", oline);
		return 1;
	}
	{
		macro_t *mac;
		arglist_t args, *argt;
		if ((mac = macrofind(ip)) != NULL) {
			if (mac->active)
				return 0;
			args.argc = 0;
			if (argp != NULL)
				while (!(ctype(*argp) & (CT_NL | CT_NUL))) {
					s = argp;
					while (!(ctype(*argp) & (CT_NL | CT_NUL)) && *argp != ',')
						++argp;
					args.argv[args.argc] = strldup(s, argp - s);
					if (args.argv[args.argc] == NULL)
						errexit("strldup() failure");
					if (*argp == ',')
						++argp;
					++args.argc;
					if (args.argc == ARG_MAX)
						flerrexit("too many arguments for macro");
				}
			if (args.argc < mac->argc)
				flerrexit("too few arguments for macro");
			argt = macargs;
			macargs = &args;
			++explvl;
			++mac->active;
			mfprint(out, " nocount\n");
			_preprocess(out, mac->val, mac);
			--mac->active;
			macargs = argt;
			if (!explvl)
				mfprintf(out, " line %i", line + 1);
			for (--args.argc; args.argc >= 0; --args.argc)
				free(args.argv[args.argc]);
			return 1;
		}
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

void _preprocess(ioh_t *out, char *in, macro_t *mac) {
	char *lnstart, *lp, *ip, *argp, *next;
	int wp, r, pps;

	/* strip comments */
	{
		char *p = in;
		int iscomment = 0, nestcomment = 0;

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
					if (c[1] == '\n') {
						c += i + 1;
						++nl;
					}
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
		next = lnstart = in;
		lp = NULL;
		argp = NULL;
		r = 0;
		while (!(ctype(*next) & (CT_NUL | CT_NL)))
			++next;
		skipnl(&next);
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
				flwarn("invalid preprocessor directive");
			goto endln;
		}
		if (!(ctype(*in) & (CT_NL | CT_NUL))) {
			if (wp & WP_TSPC)
				argp = in;
			else {
				if (pps)
					flwarn("invalid preprocessor directive");
				goto endln;
			}
		}
		if ((r = ppfind(out, ip, argp, &next, mac)))
			goto endln;
		if (lp == NULL && !(wp & WP_PSPC)) {
			lp = ip;
			wp = getword(&in, &ip);
			if (ip == NULL)
				goto endln;
			argp = NULL;
			if (!(ctype(*in) & (CT_NL | CT_NUL))) {
				if (wp & WP_TSPC)
					argp = in;
				else goto endln;
			}
			r = ppfind(out, ip, argp, &next, mac);
		}
		endln:
		if (r == 2) /* endrep or endm wants us to die */
			return;
		if (pps && !r)
			flwarn("unhandled preprocessor directive");
		if (lp != NULL && r && !macro) {
			mfwrite(out, lp, idlen(lp));
			mfwrite(out, ":", 1);
		}
		in = next;
		if (!r && !ignore && !macro && !rep0 && !pps)
			ppsub(out, lnstart, mac, 0);
		mfprint(out, "\n");
		if (!explvl && !rep)
			++line;
	}
}

/* preprocess(in, ret)
 *
 * description
 *   strips comments from in (on the string itself replaces them with space)
 *   evaluates preprocessor directives and builds with them preprocessed data
 *   that will be sent to out i/o handle
 *
 */
void preprocess(ioh_t *out, char *in) {
	file = infile;
	defargs = NULL;
	macargs = NULL;
	arr_new(&defines, sizeof(define_t));
	arr_new(&macros, sizeof(macro_t));
	line = 1;
	repno = rep0 = rep = explvl = level = ignore = 0;
	_preprocess(out, in, NULL);
	for (--defines.count; defines.count >= 0; --defines.count) {
		free(((define_t *) defines.data)[defines.count].nptr);
		if (((define_t *) defines.data)[defines.count].free)
			free(((define_t *) defines.data)[defines.count].val);
	}
	arr_free(&macros);
	arr_free(&defines);
}
