#include <stdlib.h> /* NULL */
#include "as.h" /* parseln() */
#include "mem.h"
#include "str.h" /* skipsp(), ctype(), etc */
#include "misc.h" /* flerrexit(), readfile(), getstr(), clearfile(), file */
#include "ppc.h"
#include "io.h"

arr_t defines;
arr_t macros;
arr_t files;
arr_t reps;
arr_t amacros;
arr_t adef;

arglist_t *defargs;

int level, ignore; /* depth & state of if/ifdef/ifndef directives */
int str, esc;
char *repstart, *data;

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

void strip(char *in) {
	{ /* strip comments */
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

char *sppsub(char *in, char end);

void _ppsub(ioh_t *out, char *in, define_t *parent, char end) {
	char *s, *id;
	int i;
	define_t *def;
	arglist_t args, *argt;
	amacro_t *am = arr_top(amacros, amacro_t);
	macro_t *mac = ((am == NULL) ? NULL : (macro_t *) (am->macro));

	esc = str = 0;
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
				mfprintf(out, "%ut", arr_top(reps, rep_t)->repno);
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
				p = t = sppsub(p + 1, '>');
				i = numarg(&t);
				if (*t)
					flerrexit("exess data in <> group");
				free(p);
			} else i = getval(&in);
			if (am == NULL) {
				mfprint(out, "0t");
				continue;
			}
			if (i > am->argc - mac->argc)
				flerrexit("macro wants nonexistant argument @%i", i);
			if (i == 0)
				mfprintf(out, "%ut", am->argc - mac->argc);
			else mfprint(out, am->argv[mac->argc + i - 1]);
			continue;
		}
		if (!str && *in == '[') {
			char *p;
			++in;
			p = id = sppsub(in, ']');
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
			if (am != NULL && mac != NULL && am->argc) {
				for (i = 0; i < mac->argc; ++i) {
					if (cmpid(id, mac->argv[i])) {
						_ppsub(out, am->argv[i], NULL, 0);
						break;
					}
				}
				if (i != mac->argc)
					continue;
			}

			if (parent != NULL && defargs->argc) {
				for (i = 0; i < defargs->argc; ++i) {
					if (cmpid(id, parent->argv[i])) {
						_ppsub(out, defargs->argv[i], NULL, 0);
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
					while (!(ctype(*in) & (CT_NL | CT_NUL))) {
						s = in;
						if (args.argc + 1 < def->argc)
							while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ',' && *in != ')')
								++in;
						else while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ')')
								++in;
						args.argv[args.argc] = strldup(s, in - s);
						if (args.argv[args.argc] == NULL)
							errexit("strldup() failure");
						++args.argc;
						if (args.argc == ARG_MAX)
							flerrexit("too many arguments for macro");
						if (*in == ')')
							break;
						if (*in == ',')
							++in;
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
				_ppsub(out, def->val, def, 0);
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

void ppsub(ioh_t *out, char *in, char end) {
	_ppsub(out, in, NULL, end);
}

char *sppsub(char *in, char end) {
	char *ret;
	ioh_t *h = mmemopen(0);
	if (h == NULL)
		errexit("mmemopen() failed");
	ppsub(h, in, end);
	mfwrite(h, "\0", 1);
	ret = mmemget(h);
	if (ret == NULL)
		errexit("out of memory");
	mfclose(h);
	return ret;
}

void define(char *argp, int eval) {
	define_t def, *p;
	char *s;
	if (argp == NULL)
		flerrexit("too few arguments for define directive");
	if (*argp == '<') {
		++argp;
		def.nptr = s = sppsub(argp, '>');
		while (!(ctype(*argp) & (CT_NL | CT_NUL)) && *argp != '>')
			++argp;
		if (*argp != '>')
			flerrexit("missing '>'");
		++argp;
		skipsp(&s);
		def.name = getid(&s);
		skipsp(&s);
		if (def.name == NULL || *s)
			flerrexit("syntax error on define directive");
	} else {
		def.name = getid(&argp);
		def.nptr = NULL;
	}
	def.argc = 0;
	if  (*argp == '(') {
		++argp;
		while (1) {
			s = getid(&argp);
			skipsp(&argp);
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
			skipsp(&argp);
			if (def.argc == ARG_MAX)
				flerrexit("too many arguments for macro");
		}
		if (!skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on define directive");
	} else if (!skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
		flerrexit("syntax error on define directive");
	if (eval) {
		def.val = sppsub(argp, 0);
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

void skipblock(char *start, char *end) {
	int d = 0;
	char *s = nextln;
	run = 0;
	while (1) {
		if (!parseln(s))
			flerrexit("end of file within '%s' block", start);
		if (ip != NULL) {
			if (cmpid(ip, start))
				++d;
			if (cmpid(ip, end)) {
				if (argp != NULL)
					flerrexit("'%s' takes no arguments", end);
				if (d == 0)
					break;
				--d;
			}
		}
		if (run == 0) {
			if (amacros.count == 0)
				++line;
			s = nextln;
		}
	}
}

void macro(ioh_t *out, char *argp, int eval) {
	macro_t mac;
	char *s;

	if (ignore) {
		skipblock("macro", "endm");
		mfprintf(out, "%%line %ut", line);
		return;
	}

	if (argp == NULL)
		flerrexit("too few arguments");
	mac.active = 0;
	mac.name = getid(&argp);
	if (mac.name == NULL)
		flerrexit("syntax error on macro directive");
	mac.argc = 0;
	if (skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
		while (1) {
			s = getid(&argp);
			skipsp(&argp);
			if (s == NULL)
				flerrexit("syntax error in macro parameter list");
			mac.argv[mac.argc] = s;
			++mac.argc;
			if (ctype(*argp) & (CT_NL | CT_NUL))
				break;
			if (*argp != ',')
				flerrexit("syntax error in macro parameter list");
			++argp;
			skipsp(&argp);
			if (mac.argc == ARG_MAX)
				flerrexit("too many arguments for macro");
		}
	if (!(ctype(*argp) & (CT_NL | CT_NUL)))
		flerrexit("syntax error on macro directive");

	mac.val = nextln;
	skipblock("macro", "endm");
	mfprintf(out, "%%line %ut", line);

	{ /* add macro */
		macro_t *p;
		if ((p = macrofind(mac.name)) != NULL)
			memcpy(p, &mac, sizeof(macro_t));
		else arr_add(&macros, &mac);
	}
}

char *getida(char *in) {
	char *ret;
	if (in == NULL)
		flerrexit("too few arguments");
	ret = getid(&in);
	if (ret == NULL || !(ctype(*in) & (CT_NL | CT_NUL)))
		flerrexit("syntax error");
	return ret;
}

char *getstra(char *argp, int e) {
	char *s;
	if (argp == NULL)
		flerrexit("too few arguments");
	s = getstr(&argp, e);
	if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
		flerrexit("syntax error");
	return s;
}

int ppfind(ioh_t *out, char *ip, char *argp) {
	char *s;

	if (cmpid(ip, "endm")) {
		if (amacros.count) {
			amacro_t *mac = arr_pop(amacros, amacro_t);
			--((macro_t *) (mac->macro))->active;
			for (--mac->argc; mac->argc >= 0; --mac->argc)
				free(mac->argv[mac->argc]);
			nextln = mac->nextln;
		} else flerrexit("endm without prior macro directive");
		return 1;
	}
	if (cmpid(ip, "macro")) {
		macro(out, argp, 0);
		return 1;
	}
	if (cmpid(ip, "if")) {
		int args[ARG_MAX];
		if (argp == NULL)
			flerrexit("too few arguments for if directive");
		s = argp = sppsub(argp, 0);
		getargs(argp, args, 1, 1);
		++level;
		if (!ignore && !args[0])
			ignore = level;
		free(s);
		return 1;
	}
	if (cmpid(ip, "ifdef")) {
		s = getida(argp);
		++level;
		if (!ignore && deffind(s) == NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "ifndef")) {
		s = getida(argp);
		++level;
		if (!ignore && deffind(s) != NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "endif")) {
		if (argp != NULL)
			flerrexit("too many arguments");
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
	if (cmpid(ip, "endrep")) {
		rep_t *rep = arr_top(reps, rep_t);
		if (reps.count == 0)
			flerrexit("endrep without prior 'rep'");
		if (--rep->count > 0) {
			++rep->repno;
			nextln = rep->start;
			mfprintf(out, "%%line %ut", rep->line);
		} else --reps.count;
		return 1;
	}
	if (cmpid(ip, "rep")) {
		int args[ARG_MAX];
		s = sppsub(argp, 0);
		getargs(s, args, 1, 1);
		free(s);
		if (!args[0]) {
			int d = 0;
			s = nextln;
			run = 0;
			while (1) {
				if (!parseln(s))
					flerrexit("end of file within rep group");
				if (ip != NULL) {
					if (cmpid(ip, "rep"))
						++d;
					if (cmpid(ip, "endrep")) {
						if (argp != NULL)
							flerrexit("'endrep' takes no arguments");
						if (d == 0)
							break;
						--d;
					}
				}
				if (run == 0) {
					++line;
					s = nextln;
				}
			}
			mfprintf(out, "%%line %ut", line);
		} else {
			rep_t rep;
			rep.count = args[0];
			rep.start = nextln;
			rep.repno = 0;
			rep.line = line + 1;
			arr_add(&reps, &rep);
		}
		return 1;
	}
	if (cmpid(ip, "xdefine")) {
		define(argp, 1);
		return 1;
	}
	if (cmpid(ip, "define")) {
		define(argp, 0);
		return 1;
	}
	if (cmpid(ip, "undef")) {
		define_t *def;
		s = getida(argp);
		def = deffind(s);
		if (def != NULL) {
			if (def->free)
				free(def->val);
			free(def->nptr);
			--defines.count;
			memcpy(def, ((define_t *) defines.data + defines.count), sizeof(define_t));
		}
		return 1;
	}
	if (cmpid(ip, "msg")) {
		s = getstra(argp, 1);
		flmsg(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "error")) {
		s = getstra(argp, 1);
		flerrexit(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "warn")) {
		s = getstra(argp, 1);
		flwarn(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "include")) {
		file_t ofile;
		if (argp == NULL)
			flerrexit("too few arguments for include directive");
		s = getstra(argp, 1);
		data = readfile(s);
		free(s);
		s = getstra(argp, 0);
		if (data == NULL)
			flerrexit("failed to include file '%s'", s);
		ofile.name = file;
		ofile.nextln = nextln;
		ofile.line = line;
		arr_add(&files, &ofile);
		file = s;
		mfprintf(out, "%%file \"%s\"", file);
		nextln = data;
		line = 1;
		strip(data);
		return 1;
	}
	{ /* FIXME rewrite this shit */
		macro_t *mac;
		amacro_t am;

		if ((mac = macrofind(ip)) != NULL) {
			if (mac->active)
				return 0;
			am.argc = 0;
			if (argp != NULL)
				while (!(ctype(*argp) & (CT_NL | CT_NUL))) {
					s = argp;
					while (!(ctype(*argp) & (CT_NL | CT_NUL)) && *argp != ',')
						++argp;
					am.argv[am.argc] = strldup(s, argp - s);
					if (am.argv[am.argc] == NULL)
						errexit("strldup() failure");
					if (*argp == ',')
						++argp;
					++am.argc;
					if (am.argc == ARG_MAX)
						flerrexit("too many arguments for macro");
				}
			if (am.argc < mac->argc)
				flerrexit("too few arguments for macro");
			am.nextln = nextln;
			am.line = line;
			am.macro = mac;
			arr_add(&amacros, &am);
			++mac->active;
			nextln = mac->val;
			return 1;
		}
	}
	return 0;
}

void _preprocess(ioh_t *out, char *in) {
	int r;
	file_t *f;
	strip(in);
	proceed:
	run = 0;
	while (parseln(in)) {
		r = 0;
		if (ip != NULL) {
			if ((r = ppfind(out, ip, argp))) {
				if (lp != NULL) {
					mfwrite(out, lp, idlen(lp));
					mfwrite(out, ":", 1);
				}
				run = 0;
				if (r == 2) /* endrep or endm wants us to die */
					return;
			}
		}
		if (prefix && !r)
			flwarn("unhandled preprocessor directive");
		if (run == 0) {
			if (!r && !ignore && !prefix)
				ppsub(out, in, 0);
			mfprint(out, "\n");
			++line;
			in = nextln;
		}
	}
	if (reps.count)
		flerrexit("expected 'endrep' before EOF");
	if ((f = arr_pop(files, file_t)) != NULL) {
		free(file);
/*		free(data); */
		file = f->name;
		in = f->nextln;
		line = f->line;
		mfprintf(out, "%%file \"%s\"\n", file);
		mfprintf(out, "%%line %ut\n", line + 1);
		goto proceed;
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
	define_t *def;
	file = infile;
	defargs = NULL;
	arr_new(&defines, sizeof(define_t));
	arr_new(&macros, sizeof(macro_t));
	arr_new(&files, sizeof(file_t));
	arr_new(&reps, sizeof(rep_t));
	arr_new(&amacros, sizeof(amacro_t));
	line = 1;
	level = ignore = 0;
	_preprocess(out, in);
	for (--defines.count; defines.count >= 0; --defines.count) {
		def = (define_t *) defines.data + defines.count;
		free(def->nptr);
		if (def->free)
			free(def->val);
	}
	arr_free(&amacros);
	arr_free(&reps);
	arr_free(&files);
	arr_free(&macros);
	arr_free(&defines);
}
