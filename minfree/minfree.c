#include <sys/sysinfo.h> /* sysinfo() */
#include <unistd.h> /* usleep() */
#include <stdio.h> /* stderr, fprintf() */
#include <stdlib.h> /* exit(), EXIT_FAILURE */

/* opendir(), readdir(), closedir() */
#include <sys/types.h>
#include <dirent.h>

/* open(), read(), close() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* kill() */
#include <sys/types.h>
#include <signal.h>

useconds_t interval = 100000;
int minfree = 100*1024*1024; /* minimum free space in bytes */
char *pid_pfx = "/proc/";
char *pid_sfx = "/statm";
char *proc_path = "/proc";

signed char isnum[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

void openproc(DIR **proc) {
	if (*proc != NULL)
		closedir(*proc);
	*proc = opendir(proc_path);
	if (*proc == NULL) {
		fprintf(stderr, "can't open /proc\n");
		exit(EXIT_FAILURE);
	}
}

#define scpy(dst, src, pos)  { pos = 0; scat(dst, src, pos) }
#define scat(dst, src, pos)  { int i = 0; while ((dst[pos] = src[i]) != 0) ++pos, ++i; }

int stonum(char *str) { /* probly faster */
	int i = 1, num = 0;
	signed char x;
	if ((x = isnum[(int) str[0]]) == -1) return 0;
	num += x;
	while ((x = isnum[(int) str[i]]) != -1)
		num = num * 10 + x, ++i;
	return num;
}

int main(int argc, char *argv[]) {
	struct sysinfo info;
	struct dirent *entry;
	DIR *proc = NULL;
	int bpos, i, fd, target_pid, target_size;
	char buf[512]; /* only needed to read a number, 512 bytes oughta be enough */

	openproc(&proc); /* open /proc so it is ready when we need it */
	while (1) {
		sysinfo(&info);
		if ((info.freeram + info.bufferram) < minfree/info.mem_unit) {
			/* too few memory is available, we quickly find which process to kill */
			target_size = 0;
			while((entry = readdir(proc)) != NULL) if (isnum[(int) *entry->d_name] != -1) {
				scpy(buf, pid_pfx, bpos);
				scat(buf, entry->d_name, bpos);
				scat(buf, pid_sfx, bpos);
				fd = open(buf, O_RDONLY);
				if (fd < 0) continue;
				read(fd, buf, sizeof(buf) - 1);
				close(fd);
				i = stonum(buf);
				if (i > target_size) {
					target_size = i;
					target_pid = stonum(entry->d_name);
				}
			}
			kill(target_pid, 9);
			printf("target %d annihilated\n", target_pid);
			openproc(&proc); /* close and re-open /proc */
		}
		usleep(interval);
	}
}

