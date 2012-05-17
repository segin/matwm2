#include <stdlib.h> /* malloc(), realloc(), free(), NULL */
#include <string.h> /* strlen(), memcpy() */
#include <stdarg.h>

#include "io.h"

char hex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};

int mfread(ioh_t *h, char *data, int len) {
	if (h == NULL)
		return -1;
	if (h->read == NULL)
		return -1;
	return h->read(h, data, len);
}

int mfwrite(ioh_t *h, char *data, int len) {
	if (h == NULL)
		return -1;
	if (h->write == NULL)
		return -1;
	while (len--) {
		if (*data == '\n') {
			if (h->options & MFO_DOSNL)
				h->buf[h->pos++] = '\r';
			h->buf[h->pos++] = '\n';
			if (mfflush(h) != 0)
				return -1;
			++data;
			continue;
		}
		h->buf[h->pos++] = *(data++);
		if (h->pos >= sizeof(h->buf) - 2) /* leave 2 for newline always */
			if (mfflush(h) != 0)
				return -1;
	}
	return 0;
}

int mfseek(ioh_t *h, int off, int whence) {
	if (h == NULL)
		return -1;
	if (h->seek == NULL)
		return -1;
	return h->seek(h, off, whence);
}

int mfflush(ioh_t *h) {
	if (h == NULL)
		return -1;
	if (h->write(h, h->buf, h->pos) < 0)
		return -1;
	h->pos = 0;
	return 0;
}

int mftrunc(ioh_t *h, int len) {
	if (h == NULL)
		return -1;
	if (h->trunc(h, len) < 0)
		return -1;
	h->pos = 0;
	return 0;
}

void mfclose(ioh_t *h) {
	if (h == NULL)
		return;
	mfflush(h);
	if (h->close != NULL)
		h->close(h);
	free(h->data);
	free((void *) h);
}

int mfprint(ioh_t *h, char *data) {
	return mfwrite(h, data, strlen(data));
}

int mfprintsnum(ioh_t *h, signed long long n, int b, int p) {
	if (n < 0) {
		mfwrite(h, "-", 1);
		n = -n;
	}
	return mfprintnum(h, n, b, p);
}

int mfprintnum(ioh_t *h, unsigned long long n, int b, int p) {
	char rev[64];
	int pos = sizeof(rev);
	do {
		rev[--pos] = hex[n % b];
		n /= b;
		--p;
	} while (n || p > 0);
	mfwrite(h, rev + pos, sizeof(rev) - pos);
	return 0;
}

int mfprintf(ioh_t *h, char *fmt, ...) {
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = mvafprintf(h, fmt, ap);
	va_end(ap);
	return r;
}

signed long long mprintf_getsnarg(int l, va_list ap) {
	switch (l) {
		case 0:
			return va_arg(ap, signed int);
		case 1:
			return va_arg(ap, signed long int);
		case 2:
			return va_arg(ap, signed long long int);
		default:
			return va_arg(ap, signed int);
	}
}

unsigned long long mprintf_getnarg(int l, va_list ap) {
	switch (l) {
		case 0:
			return va_arg(ap, unsigned int);
		case 1:
			return va_arg(ap, unsigned long int);
		case 2:
			return va_arg(ap, unsigned long long int);
		default:
			return va_arg(ap, unsigned int);
	}
}

int mvafprintf(ioh_t *h, char *fmt, va_list ap) {
	char *start, *s;
	unsigned long long n;
	signed long long sn;
	unsigned char c;
	int p, l;

	start = fmt;
	while (*fmt) {
		if (*fmt == '%') {
			if (mfwrite(h, start, fmt - start) < 0)
				return -1;
			++fmt;
			p = 0;
			while (*fmt >= '0' && *fmt <= '9') {
				p *= 10;
				p += *fmt - '0';
				++fmt;
			}
			l = 0;
			while (1) {
				switch (*fmt) {
					case 'l':
						++l;
						break;
					case 'q':
						l = 2;
						break;
					default:
						goto lsend;
				}
				++fmt;
			}
			lsend:
			switch (*fmt) {
				case 'c':
					c = mprintf_getnarg(l, ap);
					mfwrite(h, (char *) &c, 1);
					break;
				case 's':
					s = va_arg(ap, char *);
					if (s == NULL)
						s = "[NULL]";
					if (mfprint(h, s) < 0)
						return -1;
					break;
				case 'x':
				case 'X':
					n = mprintf_getnarg(l, ap);
					mfprintnum(h, n, 16, p);
					break;
				case 'i':
				case 'd':
					sn = mprintf_getsnarg(l, ap);
					mfprintsnum(h, sn, 10, p);
					break;
				case 'u':
					n = mprintf_getnarg(l, ap);
					mfprintnum(h, n, 10, p);
					break;
				case 'o':
					n = mprintf_getnarg(l, ap);
					mfprintnum(h, n, 8, p);
					break;
				default:
					if (mfwrite(h, fmt, 1) < 0)
						return -1;
					break;
			}
			start = ++fmt;
		} else ++fmt;
	}
	if (mfwrite(h, start, fmt - start) < 0)
		return -1;
	return 0;
}

int mfxfer(ioh_t *dst, ioh_t *src, int len) {
	int r = 0;
	if (dst == NULL || src == NULL)
		return -1;
	mfflush(dst);
	while (len > sizeof(dst->buf)) {
		len -= sizeof(dst->buf);
		r += (dst->pos = mfread(src, dst->buf, sizeof(dst->buf)));
		mfflush(dst);
	}
	r += (dst->pos = mfread(src, dst->buf, len));
	mfflush(dst);
	return r;
}

/* generic constructor */
ioh_t *_mcbopen(void *d, int len, int options,
                int (*read)(ioh_t *, char *, int),
                int (*write)(ioh_t *, char *, int),
                int (*seek)(ioh_t *, int, int),
                int (*trunc)(ioh_t *, int),
                void (*close)(ioh_t *)) {
	ioh_t *new = (ioh_t *) malloc(sizeof(ioh_t));
	if (new == NULL)
		return NULL;
	new->data = malloc(len);
	if (new->data == NULL) {
		free(new);
		return NULL;
	}
	if (d != NULL)
		memcpy(new->data, d, len);
	new->read = read;
	new->write = write;
	new->seek = seek;
	new->trunc = trunc;
	new->close = close;
	new->pos = 0;
	new->options = options;
	return new;
}

/******************
 * stdio wrappers *
 ******************/

#include <stdlib.h> /* atexit() */

ioh_t *mstdin, *mstdout, *mstderr;

void mstdio_end(void) {
	mfclose(mstdin);
	mfclose(mstdout);
	mfclose(mstderr);
}

void mstdio_init(void) {
	mstdin = mfdopen(0, 0);
	mstdout = mfdopen(1, 0);
	mstderr = mfdopen(2, 0);
	atexit(mstdio_end);
}

#include <sys/types.h> /* lseek(), ftruncate() */
#include <unistd.h>

typedef struct {
	int fd, close;
} mfddata_t;

int _mfdread(ioh_t *h, char *data, int len) {
	mfddata_t *d = (mfddata_t *) h->data;
	return read(d->fd, data, len);
}

int _mfdwrite(ioh_t *h, char *data, int len) {
	mfddata_t *d = (mfddata_t *) h->data;
	return write(d->fd, data, len);
}

void _mfdclose(ioh_t *h) {
	mfddata_t *d = (mfddata_t *) h->data;
	if (d->close)
		close(d->fd);
}

int _mfdseek(ioh_t *h, int off, int whence) {
	mfddata_t *d = (mfddata_t *) h->data;
	int wh;
	switch (whence) {
		case MSEEK_SET:
			wh = SEEK_SET;
			break;
		case MSEEK_CUR:
			wh = SEEK_CUR;
			break;
		case MSEEK_END:
			wh = SEEK_END;
			break;
		default:
			return -1;
	}
	return lseek(d->fd, off, wh);
}

int _mfdtrunc(ioh_t *h, int len) {
	mfddata_t *d = (mfddata_t *) h->data;
	return ftruncate(d->fd, len);
}

ioh_t *mfdopen(int fd, int close) {
	mfddata_t d;
	d.fd = fd;
	d.close = close;
	return _mcbopen(&d, sizeof(mfddata_t), 0, &_mfdread, &_mfdwrite, &_mfdseek, &_mfdtrunc, &_mfdclose);
}

/*************
 * file open *
 *************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

ioh_t *mfopen(char *fn, int mode) {
	int fd, o = 0;
	if (mode & MFM_RW)
		o = O_RDWR;
	else if (mode & MFM_RD)
		o = O_RDONLY;
	else if (mode & MFM_WR)
		o = O_WRONLY;
	if (mode & MFM_CREAT)
		o |= O_CREAT;
	if (mode & MFM_TRUNC)
		o |= O_TRUNC;
	if (mode & MFM_APPEND)
		o |= O_APPEND;
	if (mode & MFM_NONBLOCK)
		o |= O_NONBLOCK;
	fd = open(fn, o, 0644);
	if (fd < 0)
		return NULL;
	return mfdopen(fd, 1);
}

/*******************
 * memory wrappers *
 *******************/

typedef struct {
	char *ptr;
	int pos, len, options;
} mmemdata_t;

int _mmemread(ioh_t *h, char *data, int len) {
	mmemdata_t *d = (mmemdata_t *) h->data;
	mfflush(h);
	if (d->ptr == NULL)
		return 0;
	if (len > d->len - d->pos)
		len = d->len - d->pos;
	memcpy((void *) ((char *) d->ptr + d->pos), data, len);
	return len;
}

int _mmemwrite(ioh_t *h, char *data, int len) {
	mmemdata_t *d = (mmemdata_t *) h->data;
	d->len += len;
	if (d->len < len)
		return -1;
	d->ptr = (char *) realloc((void *) d->ptr, d->len);
	if (d->ptr == NULL)
		return -1;
	memcpy((char *) d->ptr + d->pos, data, len);
	d->pos += len;
	return len;
}

int _mmemseek(ioh_t *h, int off, int whence) {
	mmemdata_t *d;
	d = (mmemdata_t *) h->data;
	mfflush(h);
	switch (whence) {
		case MSEEK_SET:
			d->pos = off;
			break;
		case MSEEK_CUR:
			d->pos += off;
			break;
		case MSEEK_END:
			d->pos = d->len - off;
			break;
	}
	return 0;
}

int _mmemtrunc(ioh_t *h, int len) {
	mmemdata_t *d;
	d = (mmemdata_t *) h->data;
	mfflush(h);
	d->len = len;
	if (d->pos > d->len)
		d->pos = d->len;
	d->ptr = (char *) realloc((void *) d->ptr, len);
	if (d->ptr == NULL)
		return -1;
	return 0;
}

void _mmemclose(ioh_t *h) {
	mmemdata_t *d = (mmemdata_t *) h->data;
	if (d->options & MMO_FREE)
		free((void *) ((mmemdata_t *) h->data)->ptr);
}

ioh_t *mmemopen(int options) {
	mmemdata_t d;
	d.ptr = NULL;
	d.pos = 0;
	d.len = 0;
	d.options = options;
	return _mcbopen((void *) &d, sizeof(mmemdata_t), 0, &_mmemread, &_mmemwrite, &_mmemseek, &_mmemtrunc, &_mmemclose);
}

char *mmemget(ioh_t *h) {
	if (h == NULL)
		return NULL;
	mfflush(h);
	return ((mmemdata_t *) h->data)->ptr;
}

int mmemlen(ioh_t *h) {
	if (h == NULL)
		return -1;
	mfflush(h);
	return ((mmemdata_t *) h->data)->len;
}

char *msprintf(char *fmt, ...) {
	ioh_t *h = mmemopen(0);
	char *ret;
	va_list ap;
	va_start(ap, fmt);
	mvafprintf(h, fmt, ap);
	va_end(ap);
	ret = mmemget(h);
	mfclose(h);
	return ret;
}
