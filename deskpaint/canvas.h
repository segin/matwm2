#include <X11/Xlib.h>

typedef struct {
  Display *dpy;
  Window window;
  int screen, width, height, last_x, last_y;
  Pixmap buffer;
} canvas;

canvas canvas_create(Display *display, int screen, Window window, int width, int height);
void canvas_resize(canvas *c, int new_width, int new_height, GC background_gc);
void canvas_clear(canvas *c, GC gc);
void canvas_addpoint(canvas *c, int x, int y, GC gc, int s);

