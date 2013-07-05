#ifndef __DRAW_H__
#define __DRAW_H__

void draw_init(void);
void draw_dot(int x, int y, int color);
void draw_line(int x0, int y0, int x1, int y1, int color);
void draw_text(int x, int y, char *str);

#endif /* __DRAW_H__ */

