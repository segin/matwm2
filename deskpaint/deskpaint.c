#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#define DEF_FG "white"
#define DEF_BG "black"
#define DEF_LW 2
#define DEF_ELW 10

enum modes { NORMAL, ERASE };

Display *dpy;
int screen, width, height, lx, ly;
Window root;
Pixmap canvas;
GC gc, bgc;

void addpoint(int x, int y, GC gc, int s) {
  XDrawLine(dpy, canvas, gc, s ? x : lx, s ? y : ly, x, y);
  XDrawLine(dpy, root, gc, s ? x : lx, s ? y : ly, x, y);
  lx = x;
  ly = y;
}

void clear() {
  XFillRectangle(dpy, canvas, bgc, 0, 0, width, height);
  XClearWindow(dpy, root);
}

int main(int argc, char *argv[]) {
  char *dn = NULL, *fg = DEF_FG, *bg = DEF_BG, mode = NORMAL;
  int i, lw = DEF_LW, elw = DEF_ELW;
  XEvent ev;
  XGCValues gv;
  XColor c, dc;
  for(i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-display") == 0 && i + 1 < argc) {
      dn = argv[i + 1];
      i++;
      continue;
    }
    if(strcmp(argv[i], "-fg") == 0 && i + 1 < argc) {
      fg = argv[i + 1];
      i++;
      continue;
    }
    if(strcmp(argv[i], "-bg") == 0 && i + 1 < argc) {
      bg = argv[i + 1];
      i++;
      continue;
    }
    if(strcmp(argv[i], "-lw") == 0 && i + 1 < argc) {
      lw = strtol(argv[i + 1], NULL, 0);
      i++;
      continue;
    }
    if(strcmp(argv[i], "-elw") == 0 && i + 1 < argc) {
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
  gv.line_width = lw;
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), fg, &c, &dc);
  gv.foreground = c.pixel;
  gv.background = c.pixel;
  gv.cap_style = CapRound;
  gc = XCreateGC(dpy, root, GCLineWidth | GCForeground | GCCapStyle, &gv);
  gv.line_width = elw;
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), bg, &c, &dc);
  gv.foreground = c.pixel;
  bgc = XCreateGC(dpy, root, GCLineWidth | GCForeground | GCCapStyle, &gv);
  width = DisplayWidth(dpy, screen);
  height = DisplayHeight(dpy, screen);
  canvas = XCreatePixmap(dpy, root, width, height, DefaultDepth(dpy, screen));
  XSelectInput(dpy, root, ButtonPressMask | ExposureMask);
  XSetWindowBackgroundPixmap(dpy, root, canvas);
  clear();
  while(1) {
    XNextEvent(dpy, &ev);
    switch(ev.type) {
      case ButtonPress:
        if(ev.xbutton.button == Button2) {
          clear();
          break;
        } else if(ev.xbutton.button == Button1)
          mode = NORMAL;
        else if(ev.xbutton.button == Button3)
          mode = ERASE;
        else break;
        XGrabPointer(dpy, root, True, PointerMotionMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        addpoint(ev.xbutton.x, ev.xbutton.y, mode == ERASE ? bgc : gc, 1);
        break;
      case MotionNotify:
        addpoint(ev.xmotion.x, ev.xmotion.y, mode == ERASE ? bgc : gc, 0);
        break;
      case ButtonRelease:
        if(ev.xbutton.button == Button1 || ev.xbutton.button == Button3)
          XUngrabPointer(dpy, CurrentTime);
        break;
    }
  }
}

