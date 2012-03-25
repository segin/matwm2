#ifndef __IO_H__
#define __IO_H__

#include <stdarg.h>

typedef struct ioh_t ioh_t;

struct ioh_t {
	int (*read)(ioh_t *, char *, int);
	int (*write)(ioh_t *, char *, int);
	int (*seek)(ioh_t *, int, int);
	int (*trunc)(ioh_t *, int);
	void (*close)(ioh_t *);
	void *data;
	char buf[2048];
	int pos;
	int options;
};

#define MFO_DOSNL 1

extern int mfread(ioh_t *h, char *data, int len);
extern int mfwrite(ioh_t *h, char *data, int len);
extern int mfflush(ioh_t *h);
extern int mftrunc(ioh_t *h, int len);
extern void mfclose(ioh_t *h);
extern int mfprint(ioh_t *h, char *data);
extern int mfprintsnum(ioh_t *h, signed long long n, int b, int p);
extern int mfprintnum(ioh_t *h, unsigned long long n, int b, int p);
extern int mfprintf(ioh_t *h, char *fmt, ...);
extern int mvafprintf(ioh_t *h, char *fmt, va_list l);

#define MSEEK_SET 0
#define MSEEK_CUR 1
#define MSEEK_END 2

/* file descriptor wrappers */
extern ioh_t *mstdin, *mstdout, *mstderr;
extern void mstdio_init(void);
extern ioh_t *mfdopen(int fd, int close);
extern ioh_t *mfopen(char *fn, int mode);

#define MFM_RD 1
#define MFM_WR 2
#define MFM_RW 3
#define MFM_CREAT 4
#define MFM_TRUNC 8
#define MFM_APPEND 16

/* memory wrappers */
extern ioh_t *mmemopen(int options);
extern char *mmemget(ioh_t *h);
extern int mmemlen(ioh_t *h);
extern char *msprintf(char *fmt, ...);

/* mmemopen() options */
#define MMO_FREE 1

#endif /* __IO_H__ */
