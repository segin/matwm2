#ifndef __DIS_H__
#define __DIS_H__

#include "mem.h"

/* we store stuff in this way cause we intend later also support coff */
typedef struct {
	unsigned char value;
	unsigned int addr;
} dsym_t;

extern arr_t dsym;

extern void daddstr(char *s);
extern void daddhex(int n, int l);
extern int disassemble(char **ret);

#endif /* __DIS_H__ */

