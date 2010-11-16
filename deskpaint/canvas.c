#include "canvas.h"

canvas canvas_create(Display *display, int screen, Window window, int width, int height) {
  canvas ret;
  ret.dpy = display;
  ret.window = window;
  ret.screen = screen;
  ret.width = width;
  ret.height = height;
  ret.buffer = XCreatePixmap(display, window, width, height, DefaultDepth(display, screen));
  XSetWindowBackgroundPixmap(display, window, ret.buffer);
  return ret;
}

void canvas_resize(canvas *c, int new_width, int new_height, GC background_gc) {
  GC gc = XCreateGC(c->dpy, c->window, 0, NULL);
  Pixmap new_buffer = XCreatePixmap(c->dpy, c->window, new_width, new_height, DefaultDepth(c->dpy, c->screen));
  XFillRectangle(c->dpy, new_buffer, background_gc, 0, 0, new_width, new_height);
  XCopyArea(c->dpy, c->buffer, new_buffer, gc, 0, 0, c->width, c->height, 0, 0);
  XFreePixmap(c->dpy, c->buffer);
  c->buffer = new_buffer;
  XSetWindowBackgroundPixmap(c->dpy, c->window, c->buffer);
  c->width = new_width;
  c->height = new_height;
  XClearWindow(c->dpy, c->window);
}

void canvas_addpoint(canvas *c, int x, int y, GC gc, int s) {
  XDrawLine(c->dpy, c->buffer, gc, s ? x : c->last_x, s ? y : c->last_y, x, y);
  XDrawLine(c->dpy, c->window, gc, s ? x : c->last_x, s ? y : c->last_y, x, y);
  c->last_x = x;
  c->last_y = y;
}

void canvas_clear(canvas *c, GC gc) {
  XFillRectangle(c->dpy, c->buffer, gc, 0, 0, c->width, c->height);
  XClearWindow(c->dpy, c->window);
}

