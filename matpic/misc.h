#ifndef __MISC_H__
#define __MISC_H__

extern char *infile;
extern unsigned int address;

extern void cleanup(void);
extern void reset(void);

extern void errexit(char *fmt, ...);
extern void flerrexit(char *fmt, ...);
extern void flwarn(char *fmt, ...);
extern void flmsg(char *msg);

extern char *readfile(char *path);

extern unsigned int getval(char **src);
extern unsigned int numarg(char **src);

int countargs(char *src);
extern int getargs(char *src, int *args, int min, int max);
extern int sclen(char *in);
extern char *getstr(char **in, int esc);

#endif /* __MISC_H__ */
