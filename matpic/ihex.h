#ifndef __IHEX_H__
#define __IHEX_H__

#include "io.h"

extern void ihex_write(ioh_t *out);
extern void ihex_read(char *in);

#endif /* __IHEX_H__ */
