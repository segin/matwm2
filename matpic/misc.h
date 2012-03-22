#ifndef __MISC_H__
#define __MISC_H__

#include <stdarg.h>

extern char *infile;
extern unsigned long address;

extern void cleanup(void);
extern void reset(void);

extern void errexit(char *fmt, ...);
extern void flerrexit(char *fmt, ...);
extern void flwarn(char *fmt, ...);
extern void flmsg(char *msg);

extern char *readfile(char *path);

extern unsigned long getval(char **src);
extern unsigned long numarg(char **src);

extern int getargs(char *src, int *args, int min, int max);
extern void parseargs(char *in, char *mode, ...);
extern int sclen(char *in);
extern char *getstr(char **in);
extern char *unescape(char *in);

#endif /* __MISC_H__ */
