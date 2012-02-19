#ifndef __VSTR_H__
#define __VSTR_H__

typedef struct {
	char *data;
	int len, res;
} string_t;

void vstr_new(string_t *s);
void vstr_append(string_t *s, char *str, int len);
void vstr_free(string_t *s);

#endif /* __VSTR_H__ */

