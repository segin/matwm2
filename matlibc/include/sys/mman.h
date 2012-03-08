#ifndef __SYS_MMAN_H__
#define __SYS_MMAN_H__

#include <types.h>

extern void *mmap(void *addr, size_t len, int proto, int flags, int fd, off_t off);
extern int mprotect(const void *addr, size_t len, int proto);
extern int munmap(void *addr, size_t len);

#endif /* __SYS_MMAN_H__ */
