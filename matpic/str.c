/************************
 * generic string stuff *
 ************************/

#include "str.h"

#include <stdlib.h> /* NULL */
#include <string.h> /* strcpy() */

/* alfa[]
 *
 * description
 *   lookup table to figure what character is what sorta thing
 *   scroll down for meaning of numbers
 */
char alfa[256] = {
	32, 0,  0,  0,  0,  0,  0,  0,  0,  4,  16, 0,  0,  16, 0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	4,  0,  0,  64, 0,  64, 0,  0,  0,  0,  0,  0,  0,  0,  1,  0, /* . count as a letter, for local label etc */
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  8,
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,
	/* we consider all above 7F a letter, so to be compatible with utf-8 */
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
};

/* lower[]
 *
 * description
 *   to make a character lowercase
 */
unsigned char lower[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

char hexlookup[256] = {
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 16, 16, 16, 16, 16, 16,
	16, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17,
	16, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

char hexnib[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F',
};

/* skipsp(src)
 *
 * description
 *   advances *src until a character is found that is neither tab or space
 *
 * arguments
 *   char * src: zero terminated string
 *
 * return value
 *   int
 *   number of characters skipped
 */
int skipsp(char **src) {
	int n = 0;

	while (ctype(**src) & CT_SPC)
		++n, ++(*src);
	return n;
}

/* skipnl(src)
 *
 * description
 *   advances *src past the next newline
 *
 * arguments
 *   char * src: zero terminated string
 *
 * return value
 *   int
 *   1 if a newline was skipped
 *
 * notes
 *   if there is no newline but there are '\r' characters
 *   then *src will be advanced past those
 */
int skipnl(char **src) {
	int n = 0;

	while (**src == '\r')
		++(*src);
	if (**src == '\n')
		++(*src), ++n;
	return n;
}

/* getid(src)
 *
 * description
 *   advances *src until a non-identifier character is found
 *   identifier characters matched like this regex "^[a-zA-Z_][a-zA-Z0-9_]*"
 *
 * arguments
 *   char * src: zero terminated string
 *
 * return value
 *   char *
 *   NULL if no identifier found
 *   original value of *src otherwise
 */
char *getid(char **src) {
	char *ret = NULL;

	if (ctype(**src) & (CT_LET | CT_SEP)) {
		ret = *src;
		while (ctype(**src) & (CT_LET | CT_SEP | CT_NUM))
			++(*src);
	}
	return ret;
}

/* cmpid(idl, idr)
 *
 * description
 *   compares two strings until the first non-identifier character
 *   in case insensitive way
 *
 * arguments
 *   char *idl, *idr: 0 terminated strings to compare
 *
 * return value
 *   int
 *   amount of equal characters if all of them match
 *   otherwise 0
 */
int cmpid(char *idl, char *idr) {
	int n = 0;

	while (*idl && *idr &&
	       (ctype(*idl) & (CT_LET | CT_SEP | CT_NUM)) &&
	       (ctype(*idr) & (CT_LET | CT_SEP | CT_NUM)) &&
	       lower[(unsigned char) *idl] == lower[(unsigned char) *idr])
		++n, ++idl, ++idr;
	if(!(ctype(*idl) & (CT_LET | CT_SEP | CT_NUM)) &&
	   !(ctype(*idr) & (CT_LET | CT_SEP | CT_NUM)))
		return n;
	return 0;
}

int idlen(char *src) {
	int ret = 0;

	if (ctype(*src) & (CT_LET | CT_SEP))
		while (ctype(*src) & (CT_LET | CT_SEP | CT_NUM))
			++ret, ++src;
	return ret;
}

int getnum(char **src, signed long long *ret) {
	unsigned int c;
	signed long long r = 0;
	int n = 0, base = 10, sfx = 0, pfx = 0, neg = 0;
	char *s = *src;

	if (*s == '-') {
		neg = 1;
		++s;
	}
	if (*s == '$') {
		++s;
		while (*s == '_')
			++s;
		if (hexlookup[(unsigned char) *s] != 16) {
			base = 16;
		}
	} else if (*s == '0') {
		pfx = 1;
		switch(*++s) {
			case 'x':
			case 'X':
			case 'h':
			case 'H':
				base = 16;
				break;
			case 'b':
			case 'B':
				if (base == 16)
					goto oct0;
			case 'y':
			case 'Y':
				base = 2;
				break;
			case 'd':
			case 'D':
				if (base == 16)
					goto oct0;
			case 't':
			case 'T':
				base = 10;
				break;
			case 'o':
			case 'O':
			case 'q':
			case 'Q':
				base = 8;
				break;
			default:
				base = 8;
				goto oct0;
		}
		++s;
	} else { /* check for suffix notation */
		char *t;
		oct0:
		t = s;
		while ((c = hexlookup[(unsigned char) *t]) != 16)
			++t;
		sfx = 1;
		switch(*t) {
			case 'h':
			case 'H':
			case 'x':
			case 'X':
				base = 16;
				break;
			case 'y':
			case 'Y':
				base = 2;
				break;
			case 't':
			case 'T':
				base = 10;
				break;
			case 'o':
			case 'O':
			case 'q':
			case 'Q':
				base = 8;
				break;
			default:
				if (base == 16 || s == *src) {
					sfx = 0;
					break;
				}
				switch (*(s - 1)) {
					case 'b':
					case 'B':
						base = 2;
						break;
					case 'd':
					case 'D':
						base = 10;
						break;
					default:
						sfx = 0;
				}
		}
	}

	while ((c = hexlookup[(unsigned char) *s]) != 16) {
		if (c == 17) {
			++s;
			continue;
		}
		if (c >= base)
			goto endnum;
		r *= base;
		r += c;
		++s, ++n;
	}
	endnum:
	if (n || pfx) {
		*src = s;
		if (sfx)
			++*src;
		if (!n)
			++n;
		if (neg)
			r = -r;
	}
	*ret = r;
	return n;
}

int linelen(char *str) {
	int n = 0;
	while (!(ctype(*(str++)) & (CT_NL | CT_NUL)))
		++n;
	return n;
}

char *mstrldup(char *s, int len) {
	char *ret = malloc(len + 1);
	if (ret != NULL) {
		strncpy(ret, s, len);
		ret[len] = 0;
	}
	return ret;
}
