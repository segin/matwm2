#include "host.h"
#include "mem.h" /* BLOCK */
#include "misc.h" /* errexit() */

extern char *readfile(char *path) {
	int pos = 0, mem = 0;
	FILE *infd = stdin;
	char *ret = NULL;

	if (path != NULL) {
		infd = fopen(path, "r");
		if (infd == NULL)
			return ret;
	}

	while (!feof(infd)) {
		if (ferror(infd))
			errexit("failed to read file");
		if (pos == mem) {
			if (mem + BLOCK < mem)
				errexit("wtf integer overflow");
			ret = (char *) realloc((void *) ret, mem + BLOCK);
			mem += BLOCK;
			if (ret == NULL)
				errexit("out of memory");
		}
		pos += fread((void *) ret, 1, mem - pos, infd);
	}
	ret[pos] = 0;

	return ret;
}
