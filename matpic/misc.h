#ifndef __MISC_H__
#define __MISC_H__

extern char *file;
extern char *infile;
extern unsigned int address, line;

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

/* getword return value is a bit map with following properties */
#define WP_LOCAL 1  /* identifier preceded by "." */
#define WP_LABEL 2  /* we're certain it's a label, it ends with ':' */
#define WP_PSPC  4  /* the word is preceded by space */
#define WP_TSPC  8  /* there are trailing spaces */

#endif /* __MISC_H__ */
