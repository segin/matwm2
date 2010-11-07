#ifndef __MISC_H__
#define __MISC_H__

#include <stdlib.h>
#include <ncurses.h> /* for consistent bool typedef */

#define ALLOC_INCREMENT 512

void *_malloc(size_t size);
void *_realloc(void *ptr, size_t size);
char *string_copy(char *str);
void string_recopy(char **dst, char *str);

#endif /* __MISC_H__ */
