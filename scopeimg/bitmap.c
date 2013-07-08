#include "io.h"
#include <math.h> /* abs() */
#include <stdlib.h> /* exit(), EXIT_FAILURE */
#include <ft2build.h>
#include FT_FREETYPE_H

/* lets do some assuming */
#define bwidth 501
#define bheight 701

/* TODO: possible to overflow in bitmap buffer, this isn't really nice */

int bitmap[bwidth*bheight]; /* 0 is background color, 1-9 are plotter pens */

int colors[] = {
	/* 0xBBGGRR */
	0x000000,
	0x0000AA,
	0x00AA00,
	0xAA0000,
	0x00AAAA,
	0x999999,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
};

FT_Library library;
FT_Face face;

void fontfail(void) {
	mfprintf(mstderr, "error: freetype trouble\n");
	exit(EXIT_FAILURE);
}

void plot(int x, int y, int color, int sat) {
	int z = x + y * bwidth;
	int rc = 0;
	if (z < sizeof(bitmap)/sizeof(int) && color < sizeof(colors)/sizeof(int)) {
		rc += ((((bitmap[z] & 0xFF) * (0xFF - sat)) >> 8) & 0xFF) + ((((colors[color] & 0xFF) * sat) >> 8) & 0xFF);
		rc += ((((bitmap[z] & 0xFF00) * (0xFF - sat)) >> 8) & 0xFF00) + ((((colors[color] & 0xFF00) * sat) >> 8) & 0xFF00);
		rc += ((((bitmap[z] & 0xFF0000) * (0xFF - sat)) >> 8) & 0xFF0000) + ((((colors[color] & 0xFF0000) * sat) >> 8) & 0xFF0000);
		bitmap[z] = rc;
	}
}

void draw_init(void) {
	int i;
	for (i = 0; i < sizeof(bitmap)/sizeof(int); ++i)
		bitmap[i] = 0xFFFFFF;
	/* initialize freetype */
	if (FT_Init_FreeType(&library))
		fontfail();
	if (FT_New_Face(library, "/usr/share/fonts/truetype/DejaVuSansMono.ttf", 0, &face))
		fontfail();
	if (FT_Set_Char_Size(face, 0*64, 300, 100, 100))
		fontfail();
	if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
		fontfail();
}

void draw_dot(int x, int y, int color) {
	plot(bwidth - (x + 1), y, color, 0xFF);
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
 	while (1) {
		plot(x0, y0, color, 0xFF);
		if (x0 == x1 && y0 == y1) break;
		e2 = err * 2;
		if (e2 > -dy) {
			err = err - dy;
			x0 += sx;
		}
		if (x0 == x1 && y0 == y1) {
			plot(x0, y0, color, 0xFF);
			break;
		}
		if (e2 < dx) {
			err = err + dx;
			y0 += sy;
		}
	}
}

void draw_text(int x, int y, char *str, int color) {
	int xo, yo, xp = 0, err = 0, advance = (bwidth * 0x100) / 123;
	FT_Bitmap *cbm;
	x = bwidth - (x + 1);
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
			if (cbm->buffer[xo + yo * cbm->width] > 0)
				plot((xp >> 8) + x + xo + face->glyph->bitmap_left, y + yo - face->glyph->bitmap_top, color, cbm->buffer[xo + yo * cbm->width]);
		}
		xp += advance;
	}
}

void draw_finish(void) {
	int i;
	mprintf("P3\n501 701 255\n");
	for (i = 0; i < bwidth * bheight; ++i)
		mprintf("%d %d %d ", bitmap[i] & 0xFF, (bitmap[i] & 0xFF00) >> 8, (bitmap[i] & 0xFF0000) >> 16);
	mprintf("\n");
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}
