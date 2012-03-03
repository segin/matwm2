#ifndef __PPC_H__
#define __PPC_H__

#include "mem.h"
#include "io.h"

typedef struct {
	char *name, *val, *nptr;
	int active, argc;
	char *argv[ARG_MAX];
	int free;
} define_t;

typedef struct {
	char *name, *val;
	int active, argc;
	char *argv[ARG_MAX];
} macro_t;

typedef struct {
	char *name;
	char *nextln;
	int line;
} file_t;

typedef struct {
	char *argv[ARG_MAX];
	int argc;
} arglist_t;

extern void preprocess(ioh_t *out, char *in);

#endif /* __PPC_H__ */
