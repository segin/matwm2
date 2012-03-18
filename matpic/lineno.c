#include <string.h> /* memcpy() */
#include "lineno.h"
#include "mem.h"
#include "misc.h" /* errexit() */

arr_t lineno;
arr_t garbage;

void lineno_init(void) {
	arr_new(&lineno, sizeof(lineno_t));
	arr_new(&garbage, sizeof(void *));
}

void lineno_end(void) {
	arr_free(&lineno);
	for (--garbage.count; garbage.count >= 0; --garbage.count)
		free(*((void **) garbage.data + garbage.count));
	arr_free(&garbage);
}

void lineno_dropctx(void) {
	--lineno.count;
}

void lineno_inc(void) {
	if (arr_top(lineno, lineno_t)->mline)
		++arr_top(lineno, lineno_t)->mline;
	else ++arr_top(lineno, lineno_t)->line;
}

void lineno_set(unsigned int n) {
	if (arr_top(lineno, lineno_t)->mline)
		arr_top(lineno, lineno_t)->mline = n;
	else arr_top(lineno, lineno_t)->line = n;
}

unsigned int lineno_get(void) {
	return arr_top(lineno, lineno_t)->line;
}

void lineno_pushmacro(char *name, char *file, unsigned int n) {
	lineno_t ln;
	ln.mname = name;
	ln.mfile = lineno_getfile();
	ln.mline = lineno_get();
	ln.file = file;
	ln.line = n;
	arr_add(&lineno, &ln);
}

void lineno_pushfile(char *file, unsigned int n) {
	lineno_t ln;
	ln.line = n;
	ln.file = file;
	ln.mline = 0; /* so we know it is not a macro */
	arr_add(&lineno, &ln);
}

char *lineno_getfile(void) {
	return arr_top(lineno, lineno_t)->file;
}

lineno_t *lineno_getctx(void) {
	lineno_t *ln = malloc(sizeof(lineno_t));
	if (ln == NULL)
		errexit("out of memory");
	memcpy(ln, arr_top(lineno, lineno_t), sizeof(lineno_t));
	arr_add(&lineno, &ln);
	return ln;
}

void lineno_setctx(lineno_t *ctx) {
	lineno.count = 0;
	arr_add(&lineno, ctx);
}
