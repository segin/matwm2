/************************
 * generic string stuff *
 ************************/

#include <stdlib.h> /* NULL */

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
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
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

/* cmpid(idl, idr)
 *
 * description
 *   compares two strings until the first non-identifier character
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
	       *(idl++) == *(idr++))
		++n;
	if(!(alfa[(unsigned char) *idl] & (CT_LET | CT_SEP | CT_NUM)) &&
	   !(alfa[(unsigned char) *idr] & (CT_LET | CT_SEP | CT_NUM)))
		return n;
	return 0;
}

/************************
 * generic memory stuff *
 ************************/

#include <stdlib.h> /* realloc(), exit(), NULL, EXIT_FAILURE */
#include <string.h> /* memcpy */
#include <stdio.h>  /* fprintf(), stderr */

#define BLOCK 2048 /* amount of memory to allocate in one go */

typedef struct {
	void *data;
	int size, count, space;
} arr_t;

void arr_new(arr_t *a, int size) {
	a->data = NULL;
	a->size = size;
	a->count = 0;
	a->space = 0;
}

void arr_add(arr_t *a, void *data) {
	if (a->space < (a->count + 1) * a->size) {
		a->data = realloc(a->data, (((a->size * a->count) % BLOCK) + 1) * BLOCK);
		if (a->data == NULL) {
			fprintf(stderr, "out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
	memcpy((void *) (((char *) a->data) + (a->size * a->count)), data, a->size);
}

/*************
 * assembler *
 *************/

#include <stdlib.h> /* exit(), EXIT_FAILURE, EXIT_SUCCESS */
#include <stdio.h> /* fprintf(), stderr */

int line = 1;

typedef struct {
	int type, line;
	char oc[2];
	char *args;
} ins_t;

enum itype {
	IT_END, /* end of data */
	IT_ORG, /* org directive */
	IT_DAT, /* data directive */
	IT_INS, /* an actual instruction */
};

arr_t inss;

typedef struct {
	char *name;
	int address;
} label_t;

arr_t labels;

void errexit(char *msg) {
	fprintf(stderr, "line %d: %s\n", line, msg);
	exit(EXIT_FAILURE);
}

void assemble(char **code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));

	{ /* first pass */
		int address = 0;
		char *cur = NULL;
		label_t label;
		ins_t ins;

		while (**code) {
			 /* skip label and eat it if one is there
			 * also eat any preceding spaces before instruction
			 * return if nothing or nothing but label
			 */
			if (!skipsp(code)) {
				cur = getid(code);
				if (cur == NULL)
					errexit("malformed label");
				if (!skipsp(code)) {
					if (!(alfa[**code] & (CT_NL | CT_NUL)))
						errexit("unexpected character within label");
					goto nextline;
				}
				label.name = cur;
				label.address = address;
				arr_add(&labels, &label);
			}

			/* eat the instruction (or directive) */
			cur = getid(code);
			if (cur == NULL)
				errexit("malformed instruction");

			/* check for arguments */
			ins.args = NULL;
			if (!skipsp(code)) {
				if (!(alfa[**code] & (CT_NL | CT_NUL)))
					errexit("unexpected character within label");
				goto nextline;
			} else {
				if (alfa[**code] & (CT_NL | CT_NUL))
					goto nextline;
				/* we got arguments */
				ins.args = *code;
				while (!(alfa[**code] & (CT_NUL | CT_NL)))
					++*code;
			}

			/* find instructon/directive */
			if (cmpid(cur, "org")) {
				printf("org directive\n");
				/* fixme */
				goto nextline;
			}

			/* on to the next line */
			nextline:
			if (!skipnl(code) && **code)
				errexit("unexpected character");
			++line;
		}
	}
}

main() {
	char *code = " org 0x200\ntest pest\r\n vest\n";

	assemble(&code);

	return EXIT_SUCCESS;
}
