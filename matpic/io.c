#include <stdlib.h> /* malloc(), free() */
#include <string.h> /* strlen(), memcpy() */
#include <stdarg.h>

#include "io.h"

int mfread(ioh_t *h, char *data, int len) {
	return h->read(h, data, len);
}

int mfwrite(ioh_t *h, char *data, int len) {
	while (len--) {
		if (*data == '\n') {
			if (h->dosnl)
				h->buf[h->pos++] = '\r';
			h->buf[h->pos++] = '\n';
			if (mfflush(h) != 0)
				return -1;
			continue;
		}
		h->buf[h->pos++] = *(data++);
		if (h->pos >= sizeof(h->buf) - 2) /* leave 2 for newline always */
			if (mfflush(h) != 0)
				return -1;
	}
	return 0;
}

void mfclose(ioh_t *h) {
	mfflush(h);
	h->close(h);
}

int mfflush(ioh_t *h) {
	if (h->write(h, h->buf, h->pos) < 0)
		return -1;
	h->pos = 0;
	return 0;
}

int mfprint(ioh_t *h, char *data) {
	return mfwrite(h, data, strlen(data));
}

char hex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};

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
				case 's':
					s = va_arg(ap, char *);
					if (mfprint(h, s) < 0)
						return -1;
					break;
				case 'x':
					n = va_arg(ap, unsigned int);
					mfprintnum(h, n, 16, p);
					break;
				case 'i':
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

/******************
 * stdio wrappers *
 ******************/

#include <unistd.h>

int _mfdread(ioh_t *h, char *data, int len) {
	return read(*(int *) h->data, data, len);
}

int _mfdwrite(ioh_t *h, char *data, int len) {
	return write(*(int *) h->data, data, len);
}

void _mfdclose(ioh_t *h) {
	free(h->data);
	free((void *) h);
}

ioh_t *mfdopen(int fd) {
	ioh_t *new = (ioh_t *) malloc(sizeof(ioh_t));
	if (new == NULL)
		return NULL;
	new->data = (void *) malloc(sizeof(int));
	if (new->data == NULL)
		return NULL;
	*(int *) new->data = fd;
	new->read = &_mfdread;
	new->write = &_mfdwrite;
	new->close = &_mfdclose;
	new->pos = 0;
	new->dosnl = 0;
	return new;
}

/*******************
 * memory wrappers *
 *******************/

typedef struct {
	char *ptr;
	int pos, len;
} mmemdata_t;

int _mmemread(ioh_t *h, char *data, int len) {
	mmemdata_t *d = (mmemdata_t *) h->data;
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
	if (h->data == NULL)
		return -1;
	memcpy((char *) d->ptr + d->pos, data, len);
	d->pos += len;
	return len;
}

void _mmemclose(ioh_t *h) {
	free((void *) ((mmemdata_t *) h->data)->ptr);
	free(h->data);
	free((void *) h);
}

ioh_t *mmemopen(void) {
	ioh_t *new = (ioh_t *) malloc(sizeof(ioh_t));
	mmemdata_t d;
	if (new == NULL)
		return NULL;
	new->data = malloc(sizeof(mmemdata_t));
	if (new->data == NULL)
		return NULL;
	d.ptr = NULL;
	d.pos = 0;
	d.len = 0;
	memcpy(new->data, (void *) &d, sizeof(mmemdata_t));
	new->read = &_mmemread;
	new->write = &_mmemwrite;
	new->close = &_mmemclose;
	new->pos = 0;
	new->dosnl = 0;
	return new;
}

char *mmemget(ioh_t *h) {
	return ((mmemdata_t *) h->data)->ptr;
}

int mmemlen(ioh_t *h) {
	return ((mmemdata_t *) h->data)->len;
}
