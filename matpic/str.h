#ifndef __STR_H__
#define __STR_H__

/************************
 * generic string stuff *
 ************************/

#include "host.h" /* NULL */

extern char alfa[256];
extern char hexlookup[256];
extern char hexnib[16];

/* symbols used in above lookup table "alfa[]" */
#define CT_LET 1  /* letter    (a-z, A-Z) */
#define CT_NUM 2  /* number    (0-9) */
#define CT_SPC 4  /* space     ('\t' or ' ') */
#define CT_SEP 8  /* separator ('_') */
#define CT_NL  16 /* newline   ('\r' or '\n') */
#define CT_NUL 32 /* nul/0     ('\0') */
#define CT_PPC 64 /* # or % */

extern int skipsp(char **src);
extern int skipnl(char **src);
extern char *getid(char **src);
extern int idlen(char *src);
extern int cmpid(char *idl, char *idr);
extern int getnum(char **src, unsigned int *ret);

#endif /* __STR_H__ */
