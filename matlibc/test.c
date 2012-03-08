#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int fdprint(int fd, char *str) {
	return write(fd, str, strlen(str));
}

int print(char *str) {
	return fdprint(STDOUT_FILENO, str);
}

int main(int argc, char *argv[]) {
	int i;
	for (i = 0; i < argc; ++i) {
		print(argv[i]);
		print(" ");
	}
	print("\n");
	return argc;
}
