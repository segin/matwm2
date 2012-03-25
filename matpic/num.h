/* matnum
 *
 * Big number library for integers of fixed size.
 *
 * Originally intended to have 64bit numbers with sign (and not losing
 * a bit), and also to make possible the use of 64bit integers without
 * C99 compiler or 64bit CPU. As well as to be able to convert those
 * numbers to various formats (referring to the storage of negative
 * numbers mostly) without knowledge of the host architecture.
 *
 * The code assumes 'unsigned short' is at least 16 bits, and
 * 'unsigned long' at least 32bits.
 *
 * Copyright (c) 2012 Mattis Michel.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __NUM_H__
#define __NUM_H__

#define NUM_SIZE 4 /* # of 16bit values per number */

typedef struct {
	int flags, size;
	unsigned short value[NUM_SIZE];
} num_t;

#define NUM_FLAGS_SIGN 1
#define NUM_FLAGS_ZERO 2
#define NUM_FLAGS_OVFL 4
#define NUM_FLAGS_DIVZ 8

#define islt(a, b) isgt(b, a)
#define islte(a, b) isgte(b, a)

extern void num_set32(num_t *lval, unsigned long rval);
extern unsigned long num_get32(num_t *rval, int opts);
extern int num_iseq(num_t *lval, num_t *rval);
extern int num_isgt(num_t *lval, num_t *rval);
extern int num_isgte(num_t *lval, num_t *rval);
extern void num_add(num_t *lval, num_t *rval);
extern void num_sub(num_t *lval, num_t *rval);
extern void num_mul(num_t *lval, num_t *rval);
extern void num_div(num_t *lval, num_t *rval);
extern void num_mod(num_t *lval, num_t *rval);
extern void num_and(num_t *lval, num_t *rval);
extern void num_ior(num_t *lval, num_t *rval);
extern void num_eor(num_t *lval, num_t *rval);
extern void num_shl(num_t *lval, num_t *rval);
extern void num_shr(num_t *lval, num_t *rval);
extern void num_not(num_t *lval);
extern void num_neg(num_t *lval);

#endif /* __NUM_H__ */
