#ifndef __SCREEN_H__
#define __SCREEN_H__

typedef struct {
	int x, y;
	int width, height;
	long ewmh_strut[4];
} screen_dimensions;

#endif /* __SCREEN_H__ */
