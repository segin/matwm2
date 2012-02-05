#ifndef __ARCH_H__
#define __ARCH_H__

typedef struct {
	char *name;
	int oc, imask, atype;
} oc_t;

typedef struct {
	oc_t *ocs;
	int (*acmp)(int, int, int, int *);
} arch_t;

extern arch_t *arch;

#endif /* __ARCH_H__ */
