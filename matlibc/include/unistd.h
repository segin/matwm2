#ifndef __UNISTD_H__
#define __UNISTD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

extern int write(int fd, char *data, int len);
extern int read(int fd, char *data, int len);
extern off_t lseek(int fd, off_t off, int whence);
extern int brk(void *addr);

#ifdef __cplusplus
}
#endif

#endif /* __UNISTD_H__ */
