#ifndef __AS_H__
#define __AS_H__

#include "mem.h"
#include "lineno.h"

typedef struct label_t label_t;

struct label_t {
	char *name;
	int address;
	int local;
	int parent; /* tried to keep this as pointer first, big mistake */
};

typedef struct {
	unsigned int type;
	lineno_t *line;
	union {
		struct ins {
			char *args;
			unsigned char oc[6];
			unsigned int len, atype;
		} ins;
		struct org {
			unsigned int address;
		} org;
		struct data {
			char *args;
			unsigned int value;
		} data;
		struct file {
			char *file;
		} file;
		struct lbl {
			int lbl;
		} lbl;
	} d;
} ins_t;

enum itype {
	IT_END, /* end of data */
	IT_ORG, /* org directive */
	IT_DAT, /* data directive */
	IT_INS, /* an actual instruction */
	IT_FIL, /* change of filename */
	IT_LBL  /* last label encountered */
};

extern arr_t inss;
extern arr_t labels;
extern int llbl; /* last label */

extern char *lp, *ip, *argp, *nextln;
extern int prefix, run;

extern int parseln(char *in);
extern void assemble(char *code);

#endif /* __AS_H__ */
