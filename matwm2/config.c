#include "matwm.h"

XColor bg, ibg, fg, ifg;
int border_width, title_height, snapat;
int button1, button2, button3, button4, button5;
key key_next, key_prev, key_iconify, key_maximise, key_close, key_bottomleft, key_bottomright, key_topleft, key_topright;
XFontStruct *font;
GC gc, igc;
XrmDatabase cfg = NULL;

char *xrm_getstr(XrmDatabase db, char *opt_name, char *def) {
  char *type;
  XrmValue value;
  if(db && XrmGetResource(db, opt_name, opt_name, &type, &value) == True && strcmp(type, "String") == 0)
    return (char *) value.addr;
  return def;
}

key xrm_getkey(XrmDatabase db, char *opt_name, char *def) {
  key ret;
  ret.code = string_to_key(xrm_getstr(db, opt_name, def), &ret.mask);
  grab_key(root, ret.mask, ret.code);
  return ret;
}

int xrm_getint(XrmDatabase db, char *opt_name, int def) {
  char *type;
  XrmValue value;
  if(db && XrmGetResource(db, opt_name, opt_name, &type, &value) == True && strcmp(type, "String") == 0)
    return (int) strtol((char *) value.addr, NULL, 10);
  return def;
}

int strbuttonaction(char *str) {
  if(strcmp(str, "move") == 0)
    return BA_MOVE;
  if(strcmp(str, "resize") == 0)
    return BA_RESIZE;
  if(strcmp(str, "raise") == 0)
    return BA_RAISE;
  if(strcmp(str, "lower") == 0)
    return BA_LOWER;
  return BA_NONE;
}

int buttonaction(int button) {
  switch(button) {
    case Button1:
      return button1;
    case Button2:
      return button2;
    case Button3:
      return button3;
    case Button4:
      return button4;
    case Button5:
      return button5;
  }
  return BA_NONE;
}

KeyCode string_to_key(char *str, int *mask) {
  int p = 0, l = 0, len = strlen(str), mod;
  char t[len + 1];
  *mask = 0;
  strncpy(t, str, len + 1);
  while(p < len + 1) {
    if(t[p] == ' ' || t[p] == '\t' || t[p] == 0) {
      t[p] = 0;
      if(l) {
        mod = getmodifier(t + (p - l));
        *mask = *mask | mod;
        if(!mod) {
          return XKeysymToKeycode(dpy, XStringToKeysym(str + (p - l)));
        }
        l = 0;
      }
    } else l++;
    p++;
  }
  return 0;
}

void config_read(void) {
  XColor dummy;
  XGCValues gv;
  char *home = getenv("HOME");
  int cfglen = home ? strlen(home) + strlen(CFGFN) + 2 : 0;
  char cfgfn[cfglen];
  XrmInitialize();
  if(home) {
    strncpy(cfgfn, home, cfglen);
    strncat(cfgfn, "/", cfglen);
    strncat(cfgfn, CFGFN, cfglen);
    cfg = XrmGetFileDatabase(cfgfn);
  }
  button1 = strbuttonaction(xrm_getstr(cfg, "button1", DEF_BUTTON1));
  button2 = strbuttonaction(xrm_getstr(cfg, "button2", DEF_BUTTON2));
  button3 = strbuttonaction(xrm_getstr(cfg, "button3", DEF_BUTTON3));
  button4 = strbuttonaction(xrm_getstr(cfg, "button4", DEF_BUTTON4));
  button5 = strbuttonaction(xrm_getstr(cfg, "button5", DEF_BUTTON5));
  border_width = xrm_getint(cfg, "border_width", DEF_BW);
  snapat = xrm_getint(cfg, "snap", DEF_SNAP);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), xrm_getstr(cfg, "active.background", DEF_BG), &bg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), xrm_getstr(cfg, "inactive.background", DEF_IBG), &ibg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), xrm_getstr(cfg, "active.foreground", DEF_FG), &fg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), xrm_getstr(cfg, "inactive.foreground", DEF_IFG), &ifg, &dummy);
  font = XLoadQueryFont(dpy, xrm_getstr(cfg, "font", DEF_FONT));
  if(!font) {
    fprintf(stderr, "error: font not found\n");
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
}

