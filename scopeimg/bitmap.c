#include "io.h"
#include <math.h> /* abs() */

/* lets do some assuming */
#define width 501
#define height 701

char bitmap[width*height]; /* 0 is background color, 1-9 are plotter pens */

void draw_init(void) {
	int i;
	for (i = 0; i < sizeof(bitmap); ++i)
		bitmap[i] = 0;
}

void draw_dot(int x, int y, int color) {
	bitmap[width-(x+1)+y*width] = color + 1;
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
	int dx, dy, sx, sy, err, e2;
	x0 = width - (x0+1);
	x1 = width - (x1+1);
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	if (x0 < x1) sx = 1; else sx = -1;
	if (y0 < y1) sy = 1; else sy = -1;
	err = dx - dy;
	++color;
 	while (1) {
		bitmap[x0+y0*width] = color;
		if (x0 == x1 && y0 == y1) break;
		e2 = err * 2;
		if (e2 > -dy) {
			err = err - dy;
			x0 += sx;
		}
		if (x0 == x1 && y0 == y1) {
			bitmap[x0+y0*width] = color;
			break;
		}
		if (e2 < dx) {
			err = err + dx;
			y0 += sy;
		}
	}
}

void draw_text(int x, int y, char *str) {

}

void bitmap_write(void) {
	int i;
	mprintf("P1\n501 701\n");
	for (i = 0; i < width * height; ++i)
		mprintf("%d ", bitmap[i]);
	mprintf("\n");
}

