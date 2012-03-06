#define SYS_read 0
#define SYS_write 1
#define SYS_exit 60

extern int syscall(int n, ...);

int read(int fd, char *data, int len) {
	syscall(SYS_read, fd, data, len);
}

int write(int fd, char *data, int len) {
	syscall(SYS_write, fd, data, len);
}
