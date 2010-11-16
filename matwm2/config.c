#include "matwm.h"
#include "defaults.h"
#include <X11/keysymdef.h>

XColor bg, ibg;
int border_width, title_height;
unsigned int mousemodmask, move_button, resize_button, lower_button;
XFontStruct *font;
GC gc, igc;

void config_read(void) {
  /* this needs work! - configurability not yet implemented */
  XColor dummy;
  XColor fc, ifc;
  XGCValues gv;
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), DEF_BG, &bg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), DEF_IBG, &ibg, &dummy);
  border_width = DEF_BW;
  mousemodmask = DEF_MOUSEMOD;
  move_button = DEF_MOVE_BTN;
  resize_button = DEF_RESIZE_BTN;
  lower_button = DEF_LOWER_BTN;
  font = XLoadQueryFont(dpy, DEF_FONT);
  if(!font) {
    fprintf(stderr, "error: font not found\n");
    XCloseDisplay(dpy);
    exit(1);
  }
  title_height = font->max_bounds.ascent + font->max_bounds.descent + 2;
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), DEF_FG, &fc, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), DEF_IFG, &ifc, &dummy);
  gv.function = GXinvert;
  gv.subwindow_mode = IncludeInferiors;
  gv.line_width = 1;
  gv.font = font->fid;
  gv.function = GXcopy;
  gv.foreground = fc.pixel;
  gc = XCreateGC(dpy, root, GCFunction | GCSubwindowMode | GCLineWidth | GCForeground | GCFont, &gv);
  gv.foreground = ifc.pixel;
  igc = XCreateGC(dpy, root, GCFunction | GCSubwindowMode | GCLineWidth | GCForeground | GCFont, &gv);
  grab_keysym(root, Mod1Mask, XK_Tab);
}

