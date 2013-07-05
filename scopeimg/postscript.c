#include "io.h"

/* lets do some assuming */
#define width 501
#define height 701
#define lmargin 20

char *colors[9] = {
	"0 0 0 setrgbcolor",
	"0 0 0.5 setrgbcolor",
	"0 0.5 0 setrgbcolor",
	"0 0.5 0.5 setrgbcolor",
	"1 0 0 setrgbcolor",
	"0.5 0.5 0.5 setrgbcolor",
	"0.1 0.1 0.1 setrgbcolor",
	"0.5 0.5 0.5 setrgbcolor",
	"0 0 0 setrgbcolor",
};

void draw_init(void) {
	mprintf("%%!PS-Adobe-3.0 EPSF-3.0\n%%%%BoundingBox: 0 0 541 741\n1 setlinewidth\n/ComputerModern findfont\n6.8 scalefont\nsetfont\n");
}

void draw_dot(int x, int y, int color) {
	mprintf("%s\n", (color < 9) ? colors[color] : colors[0]);
	mprintf("newpath %d %d 0.5 0 360 arc fill\n", lmargin + (width - (x + 1)), height - (y + 1));
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
	mprintf("%s\n", (color < 9) ? colors[color] : colors[0]);
	mprintf("newpath %d %d moveto %d %d lineto stroke\n", lmargin + (width - (x0 + 1)), height - (y0 + 1), lmargin + (width - (x1 + 1)), height - (y1 + 1));
}

void draw_text(int x, int y, char *str, int color) {
	mprintf("%s\n", (color < 9) ? colors[color] : colors[0]);
	mprintf("newpath %d %d moveto (%s) show\n", lmargin + (width - (x + 1)), height - (y + 1), str);
}

