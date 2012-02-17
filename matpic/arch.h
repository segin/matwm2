#ifndef __ARCH_H__
#define __ARCH_H__

#include "host.h" /* NULL (used by some who include us) */
#include "as.h" /* aerrexit(), also for those who supposed to include us */

typedef struct {
	char *name;
	unsigned char oc[6], imask[6];
	int len, atype;
} oc_t;

typedef struct {
	oc_t *ocs;
	void (*acmp)(unsigned char *, int, int, int *);
	int align;
} arch_t;

extern arch_t *arch;

#endif /* __ARCH_H__ */

