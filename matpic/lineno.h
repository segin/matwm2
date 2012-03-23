#ifndef __LINENO_H__
#define __LINENO_H__

#include "mem.h"
#include "io.h"

typedef struct {
	unsigned int line;
	char *file;
	/* for macros */
	int mline;
	char *mfile, *mname;
} lineno_t;

extern arr_t lineno;
extern arr_t garbage; /* ppc uses this too */

extern void lineno_init(void);
extern void lineno_end(void);
extern void lineno_dropctx(void);
extern void lineno_inc(void);
extern void lineno_set(unsigned int n);
extern unsigned int lineno_get(void);
extern unsigned int lineno_getreal(void);
extern void lineno_pushmacro(char *name, char *file, unsigned int n);
extern void lineno_pushfile(char *file, unsigned int n, int free);
extern char *lineno_getfile(void);
extern char *lineno_getrealfile(void);
extern lineno_t *lineno_getctx(void);
extern void lineno_pushctx(lineno_t *ctx);
extern void lineno_printorigin(ioh_t *out);

#endif /* __LINENO_H__ */
