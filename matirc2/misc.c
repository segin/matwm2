#include "misc.h"
#include <string.h>

void _perror(char *str) {
	endwin();
	perror(str);
	exit(EXIT_FAILURE);	
}

void *_malloc(size_t size) {
	void *ret = malloc(size);
	if(!ret)
		_perror("malloc()");
	return ret;
}

void *_realloc(void *ptr, size_t size) {
	void *ret = realloc(ptr, size);
	if(!ret)
		_perror("realloc()");
	return ret;
}
#include "screen.h"
char *string_copy(char *str) {
	char *ret;
	int i = 0;
	if(str == NULL)
		return NULL;
	ret = (char *) _malloc(strlen(str) + 1);
	while((ret[i] = str[i]))
		i++;
	return ret;
}

void string_recopy(char **dst, char *str) {
	int i = 0;
	if(str == NULL) {
		free(*dst);
		*dst = NULL;
		return;
	}
	*dst = (char *) _realloc((void *) *dst, strlen(str) + 1);
	while(((*dst)[i] = str[i]))
		i++;
}
