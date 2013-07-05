#include "io.h"
#include <math.h> /* abs() */
#include <stdlib.h> /* exit(), EXIT_FAILURE */
#include <ft2build.h>
#include FT_FREETYPE_H

/* lets do some assuming */
#define bwidth 501
#define bheight 701

/* TODO: possible to overflow in bitmap buffer, this isn't really nice */

char bitmap[bwidth*bheight]; /* 0 is background color, 1-9 are plotter pens */

FT_Face face;

void fontfail(void) {
	mfprintf(mstderr, "error: freetype trouble\n");
	exit(EXIT_FAILURE);
}

void plot(int x, int y, int color) {
	if (x < bwidth && y < bheight)
		bitmap[x+y*bwidth] = color;
}

void draw_init(void) {
	FT_Library library;
	int i;
	for (i = 0; i < sizeof(bitmap); ++i)
		bitmap[i] = 0;

	/* initialize freetype */
	if (FT_Init_FreeType(&library))
		fontfail();
	if (FT_New_Face(library, "/usr/share/fonts/truetype/arial.ttf", 0, &face))
		fontfail();
	if (FT_Set_Char_Size(face, 0, 16*64, 300, 300))
		fontfail();

}

void draw_dot(int x, int y, int color) {
	plot(bwidth-(x + 1), y, color + 1);
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
	int dx, dy, sx, sy, err, e2;
	x0 = bwidth - (x0 + 1);
	x1 = bwidth - (x1 + 1);
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	if (x0 < x1) sx = 1; else sx = -1;
	if (y0 < y1) sy = 1; else sy = -1;
	err = dx - dy;
	++color;
 	while (1) {
		plot(x0, y0, color);
		if (x0 == x1 && y0 == y1) break;
		e2 = err * 2;
		if (e2 > -dy) {
			err = err - dy;
			x0 += sx;
		}
		if (x0 == x1 && y0 == y1) {
			plot(x0, y0, color);
			break;
		}
		if (e2 < dx) {
			err = err + dx;
			y0 += sy;
		}
	}
}

void draw_text(int x, int y, char *str, int color) {
	int xo, yo;
	FT_Bitmap cbm;
	x = bwidth - (x + 1);
	++color;
	for (; *str != 0; ++*str) {
		if (FT_Load_Char(face, *str, FT_LOAD_RENDER))
			continue;
		cbm = face->glyph->bitmap;
		for (xo = 0, yo = 0; yo < cbm.rows; ++xo) {
			if (xo == cbm.width) xo = 0, ++yo;
			plot(x + xo, y + yo, color);
		}
	}
}

void draw_finish(void) {
	int i;
	mprintf("P2\n501 701 9\n");
	for (i = 0; i < bwidth * bheight; ++i)
		mprintf("%d ", bitmap[i]);
	mprintf("\n");
}

