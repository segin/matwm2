/************************
 * generic memory stuff *
 ************************/

#include "host.h" /* realloc(), free(), NULL, memcpy() */
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
		a->data = realloc(a->data, (((a->size * a->count) % BLOCK) + 1) * BLOCK);
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

void vstr_addl(string_t *s, char *str, int len) {
	while (s->res < s->len + len + 1) {
		if (s->res + BLOCK < s->res)
			errexit("integer overflow :(");
		s->res += BLOCK;
		s->data = (char *) realloc((void *) s->data, s->res);
		if (s->data == NULL)
			errexit("out of memory");
	}
	memcpy((void *) (s->data + s->len), (void *) str, len);
	s->len += len;
	s->data[s->len] = 0;
}

void vstr_free(string_t *s) {
	s->data = NULL;
	s->len = 0;
	s->res = 0;
}

