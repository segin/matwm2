#ifndef __DRAW_H__
#define __DRAW_H__

void draw_init(void);
void draw_dot(int x, int y, int color);
void draw_line(int x1, int y1, int x2, int y2, int color);
void draw_text(int x, int y, char *str);

#endif /* __DRAW_H__ */

