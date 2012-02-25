#ifndef __PPC_H__
#define __PPC_H__

#include "mem.h"
#include "io.h"

typedef struct {
	char *name, *val;
	int active, argc;
	char *argv[ARG_MAX];
} define_t;

typedef struct {
	char *name, *val;
	int active, argc;
	char *argv[ARG_MAX];
} macro_t;

extern arr_t defines;
extern arr_t macros;

extern void preprocess(ioh_t *out, char *in);

#endif /* __PPC_H__ */
