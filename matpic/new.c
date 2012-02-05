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
	0,  0,  0,  0,  0,  0,  0,  0,  0,  4, 16,  0,  0, 16,  0,  0,
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

/* skipsp(src)
 *
 * description
 *   advances *src until a character is found that is neither tab or space
 *
 * arguments
 *   char * src: zero terminated string
 *
 * return value
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

typedef struct {
	int type;
	int line;
	char *args;
} record_t;

enum rtype {
	RT_END, /* end of data */
	RT_ORG, /* org directive */
	RT_DAT, /* data directive */
	RT_INS, /* an actual instruction */
};

arr_t *records;

typedef struct {
	char *name;
	int address;
} label_t;

arr_t *labels;

void errexit(char *msg) {
	fprintf(stderr, "error: %s\n", msg);
	exit(EXIT_FAILURE);
}

void getinstr(char **line) {
	int address = 0;
	char *cur = NULL;
	label_t label;

	/* skip label and eat it if one is there
	 * also eat any preceding spaces before instruction
	 * return if nothing or nothing but label
	 */
	if (!skipsp(line)) {
		cur = getid(line);
		if (cur == NULL)
			errexit("malformed label");
		if (!skipsp(line)) {
			if (**line && **line != '\r' && **line != '\n')
				errexit("unexpected character within label");
			return;
		}
		label.name = cur;
		label.address = address;
		arr_add(labels, &label);
	}

	/* eat the instruction */
	cur = getid(line);
	if (cur == NULL)
		errexit("malformed instruction");

	if (**line && **line != '\r' && **line != '\n')
		errexit("unexpected character");
}

main() {
	char *code = "test pest\r\n vest\n";

	arr_new(records, sizeof(record_t));

	getinstr(&code);
	skipnl(&code);
	getinstr(&code);

	return EXIT_SUCCESS;
}
