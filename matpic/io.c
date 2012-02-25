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

void mfclose(ioh_t *h) {
	if (h == NULL)
		return;
	mfflush(h);
	if (h->close != NULL)
		h->close(h);
	free(h->data);
	free((void *) h);
}

int mfflush(ioh_t *h) {
	if (h == NULL)
		return -1;
	if (h->write(h, h->buf, h->pos) < 0)
		return -1;
	h->pos = 0;
	return 0;
}

int mfprint(ioh_t *h, char *data) {
	return mfwrite(h, data, strlen(data));
}

int mfprintsnum(ioh_t *h, int n, int b, int p) {
	char rev[24]; /* best stay above 22 */
	int pos = sizeof(rev);
	if (n < 0) {
		mfwrite(h, "-", 1);
		n = -n;
	}
	do {
		rev[--pos] = hex[n % b];
		n /= b;
		--p;
	} while (n || p > 0);
	mfwrite(h, rev + pos, sizeof(rev) - pos);
	return 0;
}

int mfprintnum(ioh_t *h, unsigned int n, int b, int p) {
	char rev[24]; /* best stay above 22 */
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

int mvafprintf(ioh_t *h, char *fmt, va_list ap) {
	char *start, *s;
	unsigned int p, n;
	unsigned char c;

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
			switch (*fmt) {
				case 'c':
					c = va_arg(ap, unsigned int);
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
					n = va_arg(ap, unsigned int);
					mfprintnum(h, n, 16, p);
					break;
				case 'i':
				case 'd':
					n = va_arg(ap, int);
					mfprintsnum(h, n, 10, p);
					break;
				case 'u':
					n = va_arg(ap, unsigned int);
					mfprintnum(h, n, 10, p);
					break;
				case 'o':
					n = va_arg(ap, unsigned int);
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

/* generic constructor */
ioh_t *_mcbopen(void *d, int len, int options,
                int (*read)(ioh_t *, char *, int),
                int (*write)(ioh_t *, char *, int),
                int (*seek)(ioh_t *, int, int),
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
	new->close = close;
	new->pos = 0;
	new->options = options;
	return new;
}

/******************
 * stdio wrappers *
 ******************/

ioh_t *mstdin, *mstdout, *mstderr;

void mstdio_init(void) {
	mstdin = mfdopen(0, 0);
	mstdout = mfdopen(1, 0);
	mstderr = mfdopen(2, 0);
}

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

ioh_t *mfdopen(int fd, int close) {
	mfddata_t d;
	d.fd = fd;
	d.close = close;
	return _mcbopen(&d, sizeof(mfddata_t), 0, &_mfdread, &_mfdwrite, NULL, &_mfdclose);
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
	return _mcbopen((void *) &d, sizeof(mmemdata_t), 0, &_mmemread, &_mmemwrite, NULL, &_mmemclose);
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

void mmemtrunc(ioh_t *h) {
	mmemdata_t *d;
	if (h == NULL)
		return;
	d = (mmemdata_t *) h->data;
	d->pos = d->len = 0;
	h->pos = 0;
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
