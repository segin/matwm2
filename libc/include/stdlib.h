/* include/stdlib.h: C Standard Library functions
 * Written by Kirn Gill <segin2005@gmail.com>
 */

#ifndef __STDLIB_H__
#define __STDLIB_H__

#define ATEXIT_MAX 512

void exit(int status);
int atexit(void (*function)(void));

#ifdef INTERNAL 
void _atexitproc(void);
#endif /* INTERNAL */

#endif /* __STDLIB_H__ */