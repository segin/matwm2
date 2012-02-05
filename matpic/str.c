/************************
 * generic string stuff *
 ************************/

#include <stdlib.h> /* NULL */
#include "str.h"

/* alfa[]
 *
 * description
 *   lookup table to figure what character is what sorta thing
 *   scroll down for meaning of numbers
 */
char alfa[256] = {
	32, 0,  0,  0,  0,  0,  0,  0,  0,  4,  16, 0,  0,  16, 0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  8,
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

/* symbols used in above lookup table "alfa[]" */
#define CT_LET 1  /* letter    (a-z, A-Z) */
#define CT_NUM 2  /* number    (0-9) */
#define CT_SPC 4  /* space     ('\t' or ' ') */
#define CT_SEP 8  /* separator ('_') */
#define CT_NL  16 /* newline   ('\r' or '\n') */
#define CT_NUL 32 /* nul/0     ('\0') */

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

	while (alfa[(unsigned char) **src] & CT_SPC)
		++n, ++(*src);
	return n;
}

/* skipnl(src)
 *
 * description
 *   advances *src until a non-newline character is found
 *
 * arguments
 *   char * src: zero terminated string
 *
 * return value
 *   int
 *   number of characters skipped
 */
int skipnl(char **src) {
	int n = 0;

	while (alfa[(unsigned char) **src] & CT_NL)
		++n, ++(*src);
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

	if (alfa[(unsigned char) **src] & (CT_LET | CT_SEP)) {
		ret = *src;
		while (alfa[(unsigned char) **src] & (CT_LET | CT_SEP | CT_NUM))
			++(*src);
	}
	return ret;
}

/* lower[]
 *
 * description
 *   to make a character lowercase
 */
char lower[256] = {
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
	       (alfa[(unsigned char) *idl] & (CT_LET | CT_SEP | CT_NUM)) &&
	       (alfa[(unsigned char) *idr] & (CT_LET | CT_SEP | CT_NUM)) &&
	       lower[*(idl++)] == lower[*(idr++)])
		++n;
	if(!(alfa[(unsigned char) *idl] & (CT_LET | CT_SEP | CT_NUM)) &&
	   !(alfa[(unsigned char) *idr] & (CT_LET | CT_SEP | CT_NUM)))
		return n;
	return 0;
}

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

int getnum(char **src, unsigned int *ret) {
	unsigned int c, r = 0;
	int n = 0, base = 10;

	if (**src == '0') {
		switch(*(++*src)) {
			case 'x':
				base = 16;
				break;
			case 'b':
				base = 2;
				break;
			case 'd':
				base = 10;
				break;
			case '0':
				base = 8;
				break;
			default:
				base = 8;
				goto octal0;
		}
		++*src;
		octal0:;
	}

	while ((c = hexlookup[**src]) != 16) {
		if (c == 17)
			continue;
		if (c >= base)
			goto endnum;
		r *= base;
		r += c;
		++*src, ++n;
	}
	endnum:
	*ret = r;
	return n;
}
