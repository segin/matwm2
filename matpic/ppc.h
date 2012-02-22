#ifndef __PPC_H__
#define __PPC_H__

#include "mem.h"

typedef struct {
	char *name, *val;
	int active;
} define_t;

extern arr_t defines;

extern int preprocess(char *in, char **ret);

#endif /* __PPC_H__ */
