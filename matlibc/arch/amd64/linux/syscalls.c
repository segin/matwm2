#include <types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define SYS_read              0
#define SYS_write             1
#define SYS_open              2
#define SYS_close             3
#define SYS_stat              4
#define SYS_fstat             5
#define SYS_lstat             6
#define SYS_poll              7
#define SYS_lseek             8
#define SYS_mmap              9
#define SYS_mprotect          10
#define SYS_munmap            11
#define SYS_brk               12

#define SYS_exit 60

extern long int syscall(int n, ...);

int read(int fd, char *data, int len) {
	return syscall(SYS_read, fd, data, len);
}

int write(int fd, char *data, int len) {
	return syscall(SYS_write, fd, data, len);
}

int open(const char *path, int flags) {
	return syscall(SYS_open, path, flags);
}

int close(int fd) {
	return syscall(SYS_close, fd);
}

/* stat, fstat, lstat, poll */

off_t lseek(int fd, off_t off, int whence) {
	return syscall(SYS_lseek, fd, off, whence);
}

void *mmap(void *addr, size_t len, int proto, int flags, int fd, off_t off) {
	return (void *) syscall(SYS_mmap, addr, len, proto, flags, fd, off);
}

int mprotect(const void *addr, size_t len, int proto) {
	return syscall(SYS_mprotect, addr, len, proto);
}

int munmap(void *addr, size_t len) {
	return syscall(SYS_munmap, addr, len);
}

int brk(void *addr) {
	return syscall(SYS_brk, addr);
}

void exit(int status) {
	syscall(SYS_exit, status);
}
