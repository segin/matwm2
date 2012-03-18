#ifndef __LINENO_H__
#define __LINENO_H__

void lineno_init(void);
void lineno_end(void);
void lineno_inc(void);
void lineno_set(unsigned int n);
unsigned int lineno_get(void);
void lineno_pushmacro(char *name, char *file, int n);
void lineno_dropmacro(void);
void lineno_pushfile(char *file);
void lineno_dropfile(void);

#endif /* __LINENO_H__ */
