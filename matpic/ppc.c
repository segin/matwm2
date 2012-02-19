#include "mem.h"
#include "str.h" /* alfa[] */

arr_t defines;

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

	

/*
	*ret = out.data;
	return out.len;
*/
	return strlen(in);
}

