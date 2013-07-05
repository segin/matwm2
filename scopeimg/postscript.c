#include <stdio.h>

void draw_init(void) {
	printf("1 setlinewidth\n/Times-Roman findfont\n12 scalefont\nsetfont\n");
}

void draw_dot(double x1, double y1) {
	printf("newpath %f %f moveto %f %f lineto stroke\n", x1, y1, x2, y2);
}

void draw_line(double x1, double y1, double x2, double y2) {
	printf("newpath %f %f moveto %f %f lineto stroke\n", x1, y1, x2, y2);
}

void draw_text(double x, double y, char *str) {
	printf("newpath %f %f moveto (%s) show\n", x, y, str);
}

