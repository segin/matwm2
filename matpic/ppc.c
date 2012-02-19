#include "mem.h"
#include "str.h" /* skipsp(), alfa[], etc */
#include "misc.h" /* getword() */
#include "as.h" /* aerrexit() */

arr_t defines;

int ppfind(char *lp, char *ip, char *argp) {

	return 0;
}

int getprefix(char **src) {
	char *p = *src;
	int n = 0;
	skipsp(&p);
	while (alfa[(unsigned char) *p] & (CT_PPC))
		++p;
	if (n) {
		*src = p;
		skipsp(src);
	}
	return n;
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
	string_t out;
	int level = 0, ignore = 0; /* depth & state of if/ifdef/ifndef directives */
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

	vstr_new(&out);
	/* this very similar to assemble() */
	line = 1;
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
		if (!r && !ignore)
			vstr_addl(&out, lnstart, in - lnstart);
		++line;
	}

	*ret = out.data;
	return out.len;
}

