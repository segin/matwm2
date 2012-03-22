#ifndef __ARCH_H__
#define __ARCH_H__

#include <stdlib.h> /* NULL (used by some who include us) */
#include "misc.h"   /* flerrexit(), also for those who supposed to include us */
#include "io.h"

typedef struct {
	char *name;
	unsigned char oc[8], imask[8];
	int len, atype;
} oc_t;

typedef struct {
	oc_t *ocs;
	void (*acmp)(unsigned char *, int, int, int *);
	void (*adis)(ioh_t *out, unsigned char *oc, int atype);
	int *insord;
	int *dord;
	int align;
	int dlen;
} arch_t;

extern arch_t *arch;

#endif /* __ARCH_H__ */
