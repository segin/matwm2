#ifndef __MISC_H__
#define __MISC_H__

#include <stdarg.h>
#include "mem.h" /* string_t */

typedef signed long long sll;
typedef unsigned long long ull;

extern char *infile;
extern unsigned long address;
extern string_t inbuf;
extern string_t outbuf;

extern void cleanup(void);
extern void reset(void);

extern void errexit(char *fmt, ...);
extern void flerrexit(char *fmt, ...);
extern void flwarn(char *fmt, ...);
extern void flmsg(char *msg);

extern unsigned long long ntt(signed long long n);
extern signed long long ttn(unsigned long long n, int size);

extern void setradix(char *argp);
extern char *readfile(char *path);

extern sll getval(char **src);
extern sll numarg(char **src);

extern int getargs(char *src, sll *args, int min, int max);
extern void parseargs(char *in, char *mode, ...);
extern char *getstr(char **in);
extern char *unescape(char *in);

#endif /* __MISC_H__ */
