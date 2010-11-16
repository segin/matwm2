#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#define DEF_COLOR "white"
#define DEF_LW 2

typedef struct {
  int x, y, skip;
} point;

point *points = NULL;
int npoints = 0;
Display *dpy;
int screen;
Window root;
GC gc;

void addpoint(int x, int y, int skip) {
  points = (point *) realloc((void *) points, (npoints + 1) * sizeof(point));
  points[npoints].x = x;
  points[npoints].y = y;
  points[npoints].skip = skip;
  if(npoints && !skip)
    XDrawLine(dpy, root, gc, points[npoints - 1].x, points[npoints - 1].y, x, y);
  npoints++;
}

void clear() {
  XClearWindow(dpy, root);
  free((void *) points);
  points = NULL;
  npoints = 0;
}

void redraw() {
  int i;
  for(i = 0; i < npoints; i++) {
    if(points[i].skip)
      continue;
    XDrawLine(dpy, root, gc, points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);
  }
}

int main(int argc, char *argv[]) {
  char *dn = NULL, *color = NULL;
  int i, lw = DEF_LW;
  XEvent ev;
  XGCValues gv;
  XColor c, dc;
  for(i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-display") == 0 && i + 1 < argc) {
      dn = argv[i + 1];
      i++;
      continue;
    }
    if(strcmp(argv[i], "-color") == 0 && i + 1 < argc) {
      color = argv[i + 1];
      i++;
      continue;
    }
    if(strcmp(argv[i], "-lw") == 0 && i + 1 < argc) {
      lw = strtol(argv[i + 1], NULL, 0);
      i++;
      continue;
    }
    fprintf(stderr, "error: argument %s not recognised\n", argv[i]);
    return 1;
  }
  dpy = XOpenDisplay(dn);
  if(!dpy) {
    fprintf(stderr, "error: can't open display \"%s\"\n", XDisplayName(dn));
    exit(1);
  }
  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), color ? color : DEF_COLOR, &c, &dc);
  gv.line_width = lw;
  gv.foreground = c.pixel;
  gc = XCreateGC(dpy, root, GCLineWidth | GCForeground, &gv);
  XSelectInput(dpy, root, ButtonPressMask | ExposureMask);
  while(1) {
    XNextEvent(dpy, &ev);
    switch(ev.type) {
      case ButtonPress:
        if(ev.xbutton.button != Button1) {
          clear();
          break;
        }
        XGrabPointer(dpy, root, True, PointerMotionMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        addpoint(ev.xbutton.x, ev.xbutton.y, 1);
        break;
      case MotionNotify:
        addpoint(ev.xmotion.x, ev.xmotion.y, 0);
        break;
      case ButtonRelease:
        if(ev.xbutton.button == Button1)
          XUngrabPointer(dpy, CurrentTime);
        break;
      case Expose:
        if(ev.xexpose.count == 0)
          redraw();
        break;
    }
  }
}

