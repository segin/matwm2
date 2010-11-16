#include "matwm.h"

XColor bg, ibg, fg, ifg;
int border_width, title_height, hmaxicons, icon_width;
unsigned int mousemodmask, move_button, resize_button, raise_button, lower_button, icon_raise_button, icon_lower_button, icon_restore_button;
XFontStruct *font;
GC gc, igc;

void config_read(void) {
  /* this needs work! - configurability not yet implemented */
  XColor dummy;
  XGCValues gv;
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), DEF_BG, &bg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), DEF_IBG, &ibg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), DEF_FG, &fg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), DEF_IFG, &ifg, &dummy);
  border_width = DEF_BW;
  mousemodmask = DEF_MOUSEMOD;
  move_button = DEF_MOVE_BTN;
  resize_button = DEF_RESIZE_BTN;
  raise_button = DEF_RAISE_BTN;
  lower_button = DEF_LOWER_BTN;
  icon_raise_button = DEF_ICON_RAISE_BTN;
  icon_lower_button = DEF_ICON_LOWER_BTN;
  icon_restore_button = DEF_ICON_RESTORE_BTN;
  font = XLoadQueryFont(dpy, DEF_FONT);
  if(!font) {
    fprintf(stderr, "error: font not found\n");
    XCloseDisplay(dpy);
    exit(1);
  }
  title_height = font->max_bounds.ascent + font->max_bounds.descent + 2;
  gv.function = GXinvert;
  gv.subwindow_mode = IncludeInferiors;
  gv.line_width = 1;
  gv.font = font->fid;
  gv.function = GXcopy;
  gv.foreground = fg.pixel;
  gc = XCreateGC(dpy, root, GCFunction | GCSubwindowMode | GCLineWidth | GCForeground | GCFont, &gv);
  gv.foreground = ifg.pixel;
  igc = XCreateGC(dpy, root, GCFunction | GCSubwindowMode | GCLineWidth | GCForeground | GCFont, &gv);
  grab_keysym(root, Mod1Mask, XK_Tab);
  grab_keysym(root, Mod1Mask, XK_a);
  grab_keysym(root, Mod1Mask, XK_s);
  hmaxicons = DEF_H_ICON_COUNT;
  icon_width = (XDisplayWidth(dpy, screen) - (hmaxicons - 1)) / hmaxicons;
}

