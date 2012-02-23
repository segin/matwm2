#ifndef __HOST_H__
#define __HOST_H__

/* In hopes of keeping everything as portable as possible, here everything
 * that is used from libc, syscalls; and why it is included.
 */

#include <stdlib.h> /* NULL, realloc(), free(),
                       EXIT_FAILURE, EXIT_SUCCESS, exit() */
#include <stdio.h>  /* fread(), fopen(), fclose(), feof(), ferror() */
#include <string.h> /* memcpy(), strlen(), strcpy() */

extern char *readfile(char *path);

#endif /* __HOST_H__ */
