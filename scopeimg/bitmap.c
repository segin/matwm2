#include "io.h"
#include <math.h> /* abs() */
#include <stdlib.h> /* exit(), EXIT_FAILURE */
#include <ft2build.h>
#include FT_FREETYPE_H

/* lets do some assuming */
#define bwidth 1002
#define bheight 1402

/* TODO: possible to overflow in bitmap buffer, this isn't really nice */

char bitmap[bwidth*bheight]; /* 0 is background color, 1-9 are plotter pens */

FT_Library library;
FT_Face face;

void fontfail(void) {
	mfprintf(mstderr, "error: freetype trouble\n");
	exit(EXIT_FAILURE);
}

void plot(int x, int y, int color) {
	int z = x + y * bwidth;
	if (z < sizeof(bitmap))
		bitmap[z] = color;
}

void draw_init(void) {
	int i;
	for (i = 0; i < sizeof(bitmap); ++i)
		bitmap[i] = 0;

	/* initialize freetype */
	if (FT_Init_FreeType(&library))
		fontfail();
	if (FT_New_Face(library, "/usr/share/fonts/truetype/DejaVuSansMono.ttf", 0, &face))
		fontfail();
	if (FT_Set_Char_Size(face, 0*64, 600, 100, 100))
		fontfail();
	if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
		fontfail();
}

void draw_dot(int x, int y, int color) {
	x *= 2, y *= 2;
	plot(bwidth - x, y, color*20);
	plot(bwidth - x - 1, y, color*20);
	plot(bwidth - x, y + 1, color*20);
	plot(bwidth - x - 1, y + 1, color*20);
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
	int dx, dy, sx, sy, err, e2;
	x0 *= 2, y0 *= 2, x1 *= 2, y1 *= 2;
	x0 = bwidth - (x0 + 2);
	x1 = bwidth - (x1 + 2);
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	if (x0 < x1) sx = 1; else sx = -1;
	if (y0 < y1) sy = 1; else sy = -1;
	err = dx - dy;
	++color;
 	while (1) {
		plot(x0, y0, color*5);
		if (x0 == x1 && y0 == y1) break;
		e2 = err * 2;
		if (e2 > -dy) {
			err = err - dy;
			x0 += sx;
		}
		if (x0 == x1 && y0 == y1) {
			plot(x0, y0, color*5);
			break;
		}
		if (e2 < dx) {
			err = err + dx;
			y0 += sy;
		}
	}
}

void draw_text(int x, int y, char *str, int color) {
	int xo, yo, xp = 0;
	FT_Bitmap *cbm;
	x *= 2, y *= 2;
	x = bwidth - (x + 1);
	++color;
	for (; *str != 0; ++str) {
		if (FT_Load_Char(face, *str, FT_LOAD_RENDER))
			continue;
		cbm = &face->glyph->bitmap;
		for (xo = 0, yo = 0; yo < cbm->rows; ++xo) {
			if (xo == cbm->width) {
				xo = 0, ++yo;
				if (yo == cbm->rows)
					break;
			}
			if (cbm->buffer[xo + yo * cbm->width] > 1)
				plot(xp + x + xo + face->glyph->bitmap_left, y + yo - face->glyph->bitmap_top, cbm->buffer[xo + yo * cbm->width]/ 2);
		}
		xp += face->glyph->advance.x >> 6;
	}
}

void draw_finish(void) {
	int i;
	mprintf("P2\n1002 1402 128\n");
	for (i = 0; i < bwidth * bheight; ++i)
		mprintf("%d ", bitmap[i]);
	mprintf("\n");
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}
