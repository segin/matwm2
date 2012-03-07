#ifndef __UNISTD_H__
#define __UNISTD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

extern void *mmap(void *addr, size_t len, int proto, int flags, int fd, off_t off);
extern int mprotect(const void *addr, size_t len, int proto);
extern int munmap(void *addr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __UNISTD_H__ */
