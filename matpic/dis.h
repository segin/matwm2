#ifndef __DIS_H__
#define __DIS_H__

#include "mem.h"
#include "io.h"

/* we store stuff in this way cause we intend later also support coff */
typedef struct {
	unsigned long addr, len;
} dsym_t;

extern int format;
extern arr_t dsym;

extern void disassemble(ioh_t *out);

#endif /* __DIS_H__ */
