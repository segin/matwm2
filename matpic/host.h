#ifndef __HOST_H__
#define __HOST_H__

/* In hopes of keeping everything as portable as possible, here everything
 * that is used from libc, syscalls; and why it is included.
 */

#include <stdlib.h> /* NULL, realloc(), free(),
                       EXIT_FAILURE, EXIT_SUCCESS, exit() */
#include <stdio.h>  /* fread(), fwrite(), fopen(), fclose(), feof(), ferror(),
                       stdin, stdout, stderr */
/* Also used from stdio.h is fprintf() on platform where this are missing
 * it might be easier to change misc.c as not to use it.
 * I intend to already do this in feature release, as i expect to want my own
 * number to text function anyway.
 */

#include <string.h> /* memcpy(), strlen() */

extern char *readfile(char *path);

#endif /* __HOST_H__ */

