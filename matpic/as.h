#ifndef __AS_H__
#define __AS_H__

#include "mem.h"

#define FN_MAX 512 /* filename max length */


typedef struct label_t label_t;

struct label_t {
	char *name;
	int address;
	int local;
	int parent; /* tried to keep this as pointer first, big mistake */
};

typedef struct {
	unsigned int type, line;
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

extern char file[FN_MAX];
extern arr_t inss;
extern arr_t labels;
extern int llbl; /* last label */

void initfile(void);
void aerrexit(char *msg);
extern void assemble(char *code);

#endif /* __AS_H__ */

