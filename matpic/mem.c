/************************
 * generic memory stuff *
 ************************/

#include <stdlib.h> /* realloc(), exit(), NULL, EXIT_FAILURE */
#include <string.h> /* memcpy */
#include <stdio.h>  /* fprintf(), stderr */

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
		if (a->data == NULL) {
			fprintf(stderr, "out of memory\n");
			exit(EXIT_FAILURE);
		}
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
