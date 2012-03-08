/* include/stdlib.h: C Standard Library functions
 * Written by Kirn Gill <segin2005@gmail.com>
 */

#ifndef __STDLIB_H__
#define __STDLIB_H__

void exit(int status);
int atexit(void (*function)(void));

#ifdef INTERNAL 
#define ATEXIT_MAX 512

#if ATEXIT_MAX < 32
# warn POSIX requires atexit() to support at least 32 registrations
# warn You currently have ATEXIT_MAX configured to less than 32!
#endif

void _atexitproc(void);
#endif /* INTERNAL */

#endif /* __STDLIB_H__ */