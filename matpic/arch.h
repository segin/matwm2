#ifndef __ARCH_H__
#define __ARCH_H__

#include <stdlib.h> /* NULL (used by some who include us) */
#include "misc.h"   /* ntt() and flerrexit(), also for those who supposed to include us */
#include "io.h"

typedef struct {
	char *name;
	unsigned char oc[8], imask[8];
	int len, atype;
} oc_t;

typedef struct {
	oc_t *ocs;
	void (*acmp)(unsigned char *, int, int, signed long long *);
	void (*adis)(ioh_t *out, unsigned char *oc, int atype);
	unsigned char *mask;
	int *ord;
	int align;
} arch_t;

extern arch_t *arch;

#endif /* __ARCH_H__ */
