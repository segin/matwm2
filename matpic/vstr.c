#include "host.h" /* realloc() */
#include "mem.h" /* BLOCK */
#include "misc.h" /* errexit() */
#include "vstr.h"

void vstr_new(string_t *s) {
	s->data = NULL;
	s->len = 0;
	s->res = 0;
}

void vstr_append(string_t *s, char *str, int len) {
	while (s->res > len) {
		if (s->res + BLOCK < s->res)
			errexit("integer overflow :(");
		s->res += BLOCK;
		s->data = (char *) realloc((void *) s->data, s->res);
		if (s->data == NULL)
			errexit("out of memory");
	}
	strncpy(s->data + s->len, str, len);
	s->data[len] = 0;
	s->len += len;
}

void vstr_free(string_t *s) {
	s->data = NULL;
	s->len = 0;
	s->res = 0;
}

