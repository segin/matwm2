#include "io.h"

#define width 500
#define height 700

char bitmap[width*height]; /* 0 is background color, 1-9 are plotter pens */

void draw_init(void) {
	int i;
	for (i = 0; i < sizeof(bitmap); ++i)
		bitmap[i] = 0;
}

void draw_dot(int x, int y, int color) {
	bitmap[500*x+y] = color + 1;
}

void draw_line(int x1, int y1, int x2, int y2) {

}

void draw_text(int x, int y, char *str) {

}

void bitmap_write(void) {
	int i;
	mprintf("P1\n500 700\n");
	for (i = 0; i < width * width; ++i)
		mprintf("%d ", bitmap[i]);
	mprintf("\n");
}

