#include "matwm.h"

XColor bg, ibg;
int border_width, title_height;
unsigned int mousemodmask;
XFontStruct *font;
GC gc;

void config_read(void) {
  XColor dummy;
  XColor fc;
  XGCValues gv;
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), "blue", &bg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), "gray", &ibg, &dummy);
  border_width = 2;
  mousemodmask = Mod1Mask;
  font = XLoadQueryFont(dpy, "variable");;
  title_height = font->max_bounds.ascent + font->max_bounds.descent + 2;
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), "white", &fc, &dummy);
  gv.function = GXinvert;
  gv.subwindow_mode = IncludeInferiors;
  gv.line_width = 2;
  gv.font = font->fid;
  gv.foreground = fc.pixel;
  gv.function = GXcopy;
  gc = XCreateGC(dpy, root, GCFunction | GCSubwindowMode | GCLineWidth | GCForeground | GCFont, &gv);
}

