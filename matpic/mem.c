/************************
 * generic memory stuff *
 ************************/

#include <stdlib.h> /* realloc(), free(), NULL */
#include <string.h> /* memcpy() */
#include "misc.h" /* errexit() */
#include "mem.h"

void arr_new(arr_t *a, int size) {
	a->data = NULL;
	a->size = size;
	a->count = 0;
	a->space = 0;
}

void arr_add(arr_t *a, void *data) {
	if (a->space < (a->count + 1) * a->size) {
		a->space = (((a->size * (a->count + 1)) % BLOCK) + 1) * BLOCK;
		a->data = realloc(a->data, a->space);
		if (a->data == NULL)
			errexit("out of memory\n");
	}
	memcpy((void *) (((char *) a->data) + (a->size * a->count)), data, a->size);
	++(a->count);
}

void arr_free(arr_t *a) {
	free(a->data);
	a->data = NULL;
	a->size = 0;
	a->count = 0;
	a->space = 0;
}

void vstr_new(string_t *s) {
	s->data = NULL;
	s->len = 0;
	s->res = 0;
	vstr_add(s, ""); /* so to be sure it is not NULL */
}

void vstr_resize(string_t *s, unsigned long len) {
	s->res = len + (BLOCK - len % BLOCK);
	if (s->res < len)
		errexit("integer overflow :(");
	s->data = (char *) realloc((void *) s->data, s->res);
	if (s->data == NULL)
		errexit("out of memory");
}

void vstr_grow(string_t *s, unsigned long len) {
	len = s->len + len + 1; /* 1 extra for end 0 */
	if (len < s->len)
		errexit("integer overflow :(");
	vstr_resize(s, len);
}

void vstr_skip(string_t *s, unsigned long len) {
	vstr_grow(s, len);
	s->len += len;
}

void vstr_addl(string_t *s, char *str, unsigned long len) {
	unsigned long start = s->len;
	vstr_grow(s, len);
	memcpy(s->data + start, (void *) str, len);
	s->len += len;
	s->data[s->len] = 0;
}

void vstr_free(string_t *s) {
	free(s->data);
	s->data = NULL;
	s->len = 0;
	s->res = 0;
}
