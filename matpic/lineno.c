#include "lineno.h"
#include "mem.h"

typedef struct {
	unsigned int line;
	char *file;
} lineno_t;

typedef struct {
	unsigned int line, orgline;
	char *name, *file, *orgfile;
} lineno_macro_t;

arr_t lineno;
arr_t lineno_macro;
arr_t garbage;

void lineno_init(void) {
	arr_new(&lineno, sizeof(lineno_t));
	arr_new(&lineno_macro, sizeof(lineno_macro_t));
	arr_new(&garbage, sizeof(void *));
}

void lineno_end(void) {
	arr_free(&lineno);
	arr_free(&lineno_macro);
	arr_free(&garbage);
}

void lineno_inc(void) {
	if (lineno_macro.count)
		++arr_top(lineno_macro, lineno_macro_t)->orgline;
	else ++arr_top(lineno, lineno_t)->line;
}

void lineno_set(unsigned int n) {
	if (lineno_macro.count)
		arr_top(lineno_macro, lineno_macro_t)->orgline = n;
	else arr_top(lineno, lineno_t)->line = n;
}

unsigned int lineno_get(void) {
	return arr_top(lineno, lineno_t)->line;
}

void lineno_pushmacro(char *name, char *file, int n) {
	lineno_macro_t ln;
	ln.name = name;
	ln.orgfile = lineno_getfile();
	ln.orgline = lineno_get();
	ln.file = file;
	ln.line = n;
	arr_add(&lineno_macro, &ln);
}

void lineno_dropmacro(void) {
	--lineno_macro.count;
}

void lineno_pushfile(char *file) {
	lineno_t ln;
	ln.line = 1;
	ln.file = file;
	arr_add(&lineno, &ln);
}

void lineno_dropfile(void) {
	--lineno.count;
}

char *lineno_getfile(void) {
	return arr_top(lineno, lineno_t)->file;	
}
