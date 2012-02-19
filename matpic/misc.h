#ifndef __MISC_H__
#define __MISC_H__

extern char *infile;
extern unsigned int address, line;

void cleanup(void);
void errexit(char *msg);
void flerrexit(char *file, int line, char *msg);
void flwarn(char *file, int line, char *msg);
void fawarn(char *file, int addr, char *msg);
int getargs(char **src, int *args);
int getword(char **src, char **word);

/* getword return value is a bit map with following properties */
#define WP_LOCAL 1  /* identifier preceded by "." */
#define WP_LABEL 2  /* we're certain it's a label, it ends with ':' */
#define WP_PPC   4  /* we're certain this is a preprocessor directive it has a % or # in front of it */
#define WP_PSPC  8  /* the word is preceded by space */
#define WP_TSPC  16 /* there are trailing spaces */

#endif /* __MISC_H__ */

