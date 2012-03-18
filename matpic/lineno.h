#ifndef __LINENO_H__
#define __LINENO_H__

extern void lineno_init(void);
extern void lineno_end(void);
extern void lineno_inc(void);
extern void lineno_set(unsigned int n);
extern unsigned int lineno_get(void);
extern void lineno_pushmacro(char *name, char *file, int n);
extern void lineno_dropmacro(void);
extern void lineno_pushfile(char *file);
extern void lineno_dropfile(void);
extern char *lineno_getfile(void);

#endif /* __LINENO_H__ */
