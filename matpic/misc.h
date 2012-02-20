#ifndef __MISC_H__
#define __MISC_H__

#define LBSIZE 2048

extern char *infile;
extern unsigned int address, line;
extern int dosnl;
extern char linebuf[LBSIZE];

extern void cleanup(void);
extern void errexit(char *msg);
extern void flerrexit(char *file, int line, char *msg);
extern void flwarn(char *file, int line, char *msg);
extern void fawarn(char *file, int addr, char *msg);
extern unsigned int getval(char **src);
extern unsigned int numarg(char **src);
extern int getargs(char **src, int *args);
extern int getword(char **src, char **word);

/* getword return value is a bit map with following properties */
#define WP_LOCAL 1  /* identifier preceded by "." */
#define WP_LABEL 2  /* we're certain it's a label, it ends with ':' */
#define WP_PSPC  4  /* the word is preceded by space */
#define WP_TSPC  8  /* there are trailing spaces */

#endif /* __MISC_H__ */

