#ifndef __LINENO_H__
#define __LINENO_H__

#include "mem.h"

typedef struct {
	unsigned int line;
	char *file;
	/* for macros */
	int mline;
	char *mfile, *mname;
} lineno_t;

extern arr_t lineno;

extern void lineno_init(void);
extern void lineno_end(void);
extern void lineno_dropctx(void);
extern void lineno_inc(void);
extern void lineno_set(unsigned int n);
extern unsigned int lineno_get(void);
extern void lineno_pushmacro(char *name, char *file, unsigned int n);
extern void lineno_pushfile(char *file, unsigned int n);
extern char *lineno_getfile(void);
extern lineno_t *lineno_getctx(void);
extern void lineno_setctx(lineno_t *ctx);

#endif /* __LINENO_H__ */
