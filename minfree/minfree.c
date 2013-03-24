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

#ifndef MIN_FREE
#define MIN_FREE 128*1024 /* minimum free space in kilobytes */
#endif
#ifndef PROC_PATH
#define PROC_PATH "/proc" /* no trailing slash */
#endif
#ifndef INTERVAL
#define INTERVAL 100000 /* interval in microseconds */
#endif

useconds_t interval = INTERVAL;
int minfree = MIN_FREE;
char *pid_pfx = PROC_PATH "/";
char *pid_sfx = "/statm";
#ifndef NOPROCINFO
char *pid_sfx_status = "/status";
#endif
char *proc_path = PROC_PATH;
char *meminfo_path = PROC_PATH "/meminfo";
char *name = "minfree";

char buf[2048];

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
#define scatn(dst, src, pos) _scatn(dst, src, &(pos))

void _scatn(char *dst, int src, int *pos) {
	int m = 1;
	while (src / m > 10)
		m *= 10;
	while (m >= 1) {
		dst[*pos] = '0' + (src / m) % 10;
		m /= 10;
		++*pos;
	}
}

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

int stonum(char *str) { /* probly faster */
	int i = 1, num = 0;
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
	if (fd == 0) return 0;
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
	if (fd == 0) return 0;
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

int main(int argc, char *argv[]) {
	struct dirent *entry;
	DIR *proc = NULL;
	int bpos, i, fd, target_pid, target_size;
	char buf2[256];
	time_t t;
	struct tm *lt;
	meminfo_t minfo;
	meminfo_status_t pstatus;

	if (argc > 1)
		minfree = strtol(argv[1], NULL, 0);
	if (argc > 2)
		interval = strtol(argv[2], NULL, 0) * 1000;

	openproc(&proc); /* open /proc so it is ready when we need it */
	printf("%s started\n", name);
	printf("treshold = %d kB\ninterval = %d ms\n\n", minfree, interval / 1000);
	while (1) {
		if (meminfo_read(meminfo_path, &minfo) <= 0) {
				fprintf(stderr, "failed to read meminfo\n");
				return EXIT_FAILURE;
		}
		if ((minfo.free + minfo.buffers + minfo.cache) < minfree) {
			/* too few memory is available, we quickly find which process to kill */
			target_size = 0;
			while((entry = readdir(proc)) != NULL) if (isnum[(int) *entry->d_name] != -1) {
				scpy(buf, pid_pfx, bpos);
				scat(buf, entry->d_name, bpos);
				scat(buf, pid_sfx, bpos);
				fd = open(buf, O_RDONLY);
				if (fd < 0) continue;
				i = read(fd, buf, sizeof(buf) - 1);
				close(fd);
				if (i < 0) {
					printf("read error (%s)\n", buf);
					continue;
				}
				i = stonum(buf);
				printf("%s : %d\n", entry->d_name, i);
				if (i > target_size) {
					target_size = i;
					target_pid = stonum(entry->d_name);
				}
			}
#ifndef NOPROCINFO
			scpy(buf, pid_pfx, bpos);
			scatn(buf, target_pid, bpos);
			scat(buf, pid_sfx_status, bpos);
			fd = open(buf, O_RDONLY);
			if (fd < 0) continue;
			i = read(fd, buf, sizeof(buf) - 1);
			close(fd);
			if (i < 0) {
				i = 0;
				printf("read error (%s)\n", buf);
			}
			buf[i] = 0;
#endif
			kill(target_pid, 9);
			time(&t);
			lt = localtime(&t);
			strftime(buf2, sizeof(buf2), "%F %T", lt);
#if !defined(NOPROCINFO) && !defined(ALLPROCINFO)
			for (i = 6; buf[i] != '\n' && buf[i] != 0; ++i);
				buf[i] = 0;
			printf("%s process %d (%s) annihilated\n", buf2, target_pid, buf+6);
#else
			printf("%s process %d annihilated\n", buf2, target_pid);
#endif
#ifdef ALLPROCINFO
			printf("%s\n", buf);
#endif
			openproc(&proc); /* close and re-open /proc */
		}
		usleep(interval);
	}
}
