#include "host.h" /* realloc(), memcpy(), strlen() */
#include "mem.h" /* BLOCK */
#include "misc.h" /* errexit() */
#include "vstr.h"

void vstr_new(string_t *s) {
	s->data = NULL;
	s->len = 0;
	s->res = 0;
}

void vstr_addl(string_t *s, char *str, int len) {
	while (s->res < s->len + len) {
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

