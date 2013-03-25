/*
 * M   M  I  N   N  FFFF  RRR    EEEE EEEE
 * MM MM  I  NN  N  F     R  R   E    E
 * M M M  I  N N N  FFF   RRR    EEE  EEE
 * M   M  I  N  NN  F     R  R   E    E
 * M   M  I  N   N  F     R   R  EEEE EEEE
 *               the linux crash preventer
 *
 * Copyright (c) 2013, Mattis Michel <sic_zer0@hotmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

/* time(), ctime() */
#include <time.h>

/* strcpy(), strcat(), strerror() */
#include <string.h>

/* errno */
#include <errno.h>

#ifndef MIN_FREE
#define MIN_FREE 64*1024 /* minimum free space in kilobytes */
#endif
#ifndef PROC_PATH
#define PROC_PATH "/proc" /* no trailing slash */
#endif
#ifndef INTERVAL
#define INTERVAL 100000 /* interval in microseconds */
#endif
#ifndef MAX_LIST_ITEMS
#define MAX_LIST_ITEMS 2048
#endif

useconds_t interval = INTERVAL;
int minfree = MIN_FREE;

char *pid_pfx = PROC_PATH "/";
char *pid_sfx = "/status";
char *proc_path = PROC_PATH;
char *meminfo_path = PROC_PATH "/meminfo";
char *name = "minfree";

char buf[2048];

char list[MAX_LIST_ITEMS][128];
int list_len = 0;
int list_color = 0;
#define WHITE 1
#define BLACK 2

signed char isnum[256] = {128
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

unsigned long long stonum(char *str) { /* probly faster */
	int i = 1;
	unsigned long long num = 0;
	signed char x;
	if ((x = isnum[(int) str[0]]) == -1) return 0;
	num += x;
	while ((x = isnum[(int) str[i]]) != -1)
		num = num * 10 + x, ++i;
	return num;
}

int meminfo_isitem(char *buf, char *item) { /* make sure item is smaller than buf (!) */
	int i;
	for (i = 0;; ++i) {
		if (item[i] != buf[i] || item[i] == 0) {
			if (item[i] == 0 && buf[i] == ':')
				return i;
			return 0;
		}
	}
	return 0;
}

void meminfo_numitem(char *buf, char *item, unsigned long long *ret) {
	int x;
	if ((x = meminfo_isitem(buf, item)) > 0) {
		buf += x + 1;
		while (*buf == ' ' || *buf == '\t') ++buf;
		*ret = stonum(buf);
	}
}

void meminfo_name(char *buf, char *item, char *ret, int l) {
	int x;
	if ((x = meminfo_isitem(buf, item)) > 0) {
		buf += x + 1;
		while (*buf == ' ' || *buf == '\t') ++buf;
		while (*buf != '\n' && *buf != 0 && l-- >= 0)
			*(ret++) = *(buf++);
		*ret = 0;
	}
}

typedef struct {
	unsigned long long total;
	unsigned long long free;
	unsigned long long buffers;
	unsigned long long cache;
} meminfo_t;

typedef struct {
	char name[128];
	unsigned long long size;
} meminfo_status_t;

int meminfo_read(char *fn, meminfo_t *minfo) {
	int fd, i, j, l = 0;
	minfo->total = 0;
	minfo->free = 0;
	minfo->buffers = 0;
	minfo->cache = 0;
	fd = open(fn, O_RDONLY);
	if (fd <= 0) return 0;
	while (1) {
		if ((i = read(fd, buf + l, sizeof(buf) - l)) > 0) l += i;
		if (l <= 0) break;
		meminfo_numitem(buf, "MemTotal", &minfo->total);
		meminfo_numitem(buf, "MemFree", &minfo->free);
		meminfo_numitem(buf, "Buffers", &minfo->buffers);
		meminfo_numitem(buf, "Cached", &minfo->cache);
		for (i = 0; i < l && buf[i] != '\n'; ++i);
		if (buf[i] != '\n') break;
		++i;
		for (j = i; i < l; ++i)
			buf[i - j] = buf[i];
		l -= j;
		buf[l] = 0;
	}
	close(fd);
	return 1;
}

int meminfo_statusread(char *fn, meminfo_status_t *pstatus) {
	int fd, i, j, l = 0;
	pstatus->name[0] = 0;
	pstatus->size = 0;
	fd = open(fn, O_RDONLY);
	if (fd <= 0) return 0;
	while (1) {
		if ((i = read(fd, buf + l, sizeof(buf) - l)) > 0) l += i;
		if (l <= 0) break;
		meminfo_name(buf, "Name", pstatus->name, sizeof(pstatus->name));
		meminfo_numitem(buf, "VmSize", &pstatus->size);
		for (i = 0; i < l && buf[i] != '\n'; ++i);
		if (buf[i] != '\n') break;
		++i;
		for (j = i; i < l; ++i)
			buf[i - j] = buf[i];
		l -= j;
		buf[l] = 0;
	}
	close(fd);
	return 1;
}

int readlist(char *fn) {
	int fd, i, j, l = 0;
	fd = open(fn, O_RDONLY);
	if (fd <= 0) return 0;
	list_len = 0;
	while (1) {
		if ((i = read(fd, buf + l, sizeof(buf) - l)) > 0) l += i;
		if (l <= 0) break;
		for (i = 0; i < l && buf[i] != '\n' && i < sizeof(list[list_len]); ++i)
			list[list_len][i] = buf[i];
		++list_len;
		if (list_len > MAX_LIST_ITEMS) {
			printf("maximum list items reached\n");
			goto endreadlist;
		}
		if (i != 0) {
			printf("list item too long\n");
			for (; i < l && buf[i] != '\n' && i < sizeof(list[list_len]); ++i);
		}
		if (buf[i] != '\n') break;
		++i;
		for (j = i; i < l; ++i)
			buf[i - j] = buf[i];
		l -= j;
		buf[l] = 0;
	}
	endreadlist:;
	close(fd);
	return 1;
}

void openproc(DIR **proc) {
	if (*proc != NULL)
		closedir(*proc);
	*proc = opendir(proc_path);
	if (*proc == NULL) {
		fprintf(stderr, "can't open /proc\n");
		exit(EXIT_FAILURE);
	}
}

char tbuf[256];

char *gettime(void) {
	time_t t;
	struct tm *lt;
	time(&t);
	lt = localtime(&t);
	strftime(tbuf, sizeof(tbuf), "%F %T", lt);
	return tbuf;
}

int main(int argc, char *argv[]) {
	struct dirent *entry;
	DIR *proc = NULL;
	int bpos, i, fd, target_size;
	pid_t target_pid;
	meminfo_t minfo;
	meminfo_status_t pstatus;
	char target_name[sizeof(pstatus.name)];

	if (argc > 1)
		minfree = strtol(argv[1], NULL, 0);
	if (argc > 2)
		interval = strtol(argv[2], NULL, 0) * 1000;

	printf("%s %s started\n", gettime(), name);

	if (readlist("/etc/mfwhitelist")) {
		list_color = WHITE;
		printf("whitelist found\n");
	} else if (readlist("/etc/mfblacklist")) {
		list_color = BLACK;
		printf("blacklist found\n");
	} else printf("no white- or blacklist found, all processes targetted\n");

	printf("treshold = %d kB\ninterval = %d ms\n\n", minfree, interval / 1000);

	while (1) {
		if (meminfo_read(meminfo_path, &minfo) <= 0) {
				fprintf(stderr, "failed to read meminfo\n");
				return EXIT_FAILURE;
		}
		if ((minfo.free + minfo.buffers + minfo.cache) < minfree) {
			/* too few memory is available, we quickly find which process to kill */
			printf("memory running out, finding process to kill\n");
			openproc(&proc); /* open or re-open /proc */
			target_size = 0;
			target_pid = 0;
			while((entry = readdir(proc)) != NULL) if (isnum[(int) *entry->d_name] != -1) {
				strcpy(buf, pid_pfx);
				strcat(buf, entry->d_name);
				strcat(buf, pid_sfx);
				if (meminfo_statusread(buf, &pstatus) <= 0)
					continue;
				if (pstatus.size > target_size) {
					target_size = pstatus.size;
					target_pid = stonum(entry->d_name);
					strcpy(target_name, pstatus.name);
				}
			}
			if (meminfo_read(meminfo_path, &minfo) <= 0) {
				fprintf(stderr, "failed to read meminfo\n");
				return EXIT_FAILURE;
			}
			if ((minfo.free + minfo.buffers + minfo.cache) >= minfree) {
				printf("%s situation resolved itself somehow, cancelling\n", gettime());
				goto nokill;
			}
			if (target_pid == 0) {
				printf("%s found nothing we are allowed to kill :(\n", gettime());
				goto nokill;
			}
			if (kill(target_pid, 9) != 0) {
				printf("%s failed to kill %d (%s) (%s)", gettime(), target_pid, target_name, strerror(errno));
				goto nokill;
			}

			printf("%s process %d (%s) annihilated\n", gettime(), target_pid, target_name);
			nokill:;
			usleep(500000); /* sometimes takes a while before /proc is updated */
		}
		usleep(interval);
	}
}
