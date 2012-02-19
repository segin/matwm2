#ifndef __VSTR_H__
#define __VSTR_H__

typedef struct {
	char *data;
	int len, res;
} string_t;

#define vstr_add(s, str) (vstr_addl(s, str, strlen(str)))

void vstr_new(string_t *s);
void vstr_addl(string_t *s, char *str, int len);
void vstr_free(string_t *s);

#endif /* __VSTR_H__ */

