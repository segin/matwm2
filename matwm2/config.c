#include "matwm.h"

XColor bg, ibg, fg, ifg;
int border_width, title_height, hmaxicons, icon_width;
unsigned int mousemodmask;
char *cbutton1, *cbutton2, *cbutton3, *cbutton4, *cbutton5, *ibutton1, *ibutton2, *ibutton3, *ibutton4, *ibutton5;
KeySym key_next, key_prev, key_next_icon, key_prev_icon, key_iconify, key_maximise, key_close;
int modmask_next, modmask_prev, modmask_next_icon, modmask_prev_icon, modmask_iconify, modmask_maximise, modmask_close;
XFontStruct *font;
GC gc, igc;

char *xrm_getstr(XrmDatabase db, char *opt_name, char *def) {
  char *type;
  XrmValue value;
  if(db && XrmGetResource(db, opt_name, opt_name, &type, &value) == True && strcmp(type, "String") == 0)
    return (char *) value.addr;
  return def;
}

int xrm_getint(XrmDatabase db, char *opt_name, int def) {
  char *type;
  XrmValue value;
  if(db && XrmGetResource(db, opt_name, opt_name, &type, &value) == True && strcmp(type, "Integer") == 0)
    return *(int *) value.addr;
  return def;
}

char *buttonaction(int n, int button) {
  if(clients[n].iconic) {
    switch(button) {
      case Button1:
        return ibutton1;
      case Button2:
        return ibutton2;
      case Button3:
        return ibutton3;
      case Button4:
        return ibutton4;
      case Button5:
        return ibutton5;
    }
  } else switch(button) {
    case Button1:
      return cbutton1;
    case Button2:
      return cbutton2;
    case Button3:
      return cbutton3;
    case Button4:
      return cbutton4;
    case Button5:
      return cbutton5;
  }
  return None;
}

int keycode_to_modmask(KeyCode key) {
  int i;
  for(i = 0; i < 8; i++)
    if(modmap->modifiermap[modmap->max_keypermod * i] == key)
      return (1 << i);
  return 0;
}

KeySym string_to_key(char *str, int *mask) {
  int p = 0, l = 0, len = strlen(str), key;
  char t[len + 1];
  *mask = 0;
  strlcpy(t, str, len + 1);
  while(p < len + 1) {
    if(t[p] == ' ' || t[p] == '\t' || t[p] == 0) {
      t[p] = 0;
      if(l) {
        key = keycode_to_modmask(XKeysymToKeycode(dpy, XStringToKeysym(t + (p - l))));
        *mask = *mask | key;
        if(!key) {
          return XStringToKeysym(str + (p - l));
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
  char *home = getenv("HOME"), *bgc, *ibgc, *fgc, *ifgc, *fontc;
  int cfglen = home ? strlen(home) + strlen(CFGFN) + 2 : 0;
  char cfgfn[cfglen];
  XrmDatabase cfg = NULL;
  XrmInitialize();
  if(home) {
    strlcpy(cfgfn, home, cfglen);
    strlcat(cfgfn, "/", cfglen);
    strlcat(cfgfn, CFGFN, cfglen);
    cfg = XrmGetFileDatabase(cfgfn);
  }
  bgc = xrm_getstr(cfg, "active.background", DEF_BG);
  ibgc = xrm_getstr(cfg, "inactive.background", DEF_IBG);
  fgc = xrm_getstr(cfg, "active.foreground", DEF_FG);
  ifgc = xrm_getstr(cfg, "inactive.foreground", DEF_IFG);
  cbutton1 = xrm_getstr(cfg, "client_button1", DEF_CBUTTON1);
  cbutton2 = xrm_getstr(cfg, "client_button2", DEF_CBUTTON2);
  cbutton3 = xrm_getstr(cfg, "client_button3", DEF_CBUTTON3);
  cbutton4 = xrm_getstr(cfg, "client_button4", DEF_CBUTTON4);
  cbutton5 = xrm_getstr(cfg, "client_button5", DEF_CBUTTON5);
  ibutton1 = xrm_getstr(cfg, "icon_button1", DEF_IBUTTON1);
  ibutton2 = xrm_getstr(cfg, "icon_button2", DEF_IBUTTON2);
  ibutton3 = xrm_getstr(cfg, "icon_button3", DEF_IBUTTON3);
  ibutton4 = xrm_getstr(cfg, "icon_button4", DEF_IBUTTON4);
  ibutton5 = xrm_getstr(cfg, "icon_button5", DEF_IBUTTON5);
  fontc = xrm_getstr(cfg, "font", DEF_FONT);
  border_width = xrm_getint(cfg, "border_width", DEF_BW);
  hmaxicons = xrm_getint(cfg, "icons_per_line", DEF_H_ICON_COUNT);
  string_to_key(xrm_getstr(cfg, "mouse_modifier", DEF_MOUSEMOD), &mousemodmask);
  key_next = string_to_key(xrm_getstr(cfg, "key_next", DEF_KEY_NEXT), &modmask_next);
  key_prev = string_to_key(xrm_getstr(cfg, "key_prev", DEF_KEY_PREV), &modmask_prev);
  key_next_icon = string_to_key(xrm_getstr(cfg, "key_next_icon", DEF_KEY_NEXT_ICON), &modmask_next_icon);
  key_prev_icon = string_to_key(xrm_getstr(cfg, "key_prev_icon", DEF_KEY_PREV_ICON), &modmask_prev_icon);
  key_iconify = string_to_key(xrm_getstr(cfg, "key_iconify", DEF_KEY_ICONIFY), &modmask_iconify);
  key_close = string_to_key(xrm_getstr(cfg, "key_close", DEF_KEY_CLOSE), &modmask_close);
  grab_keysym(root, modmask_next, key_next);
  grab_keysym(root, modmask_prev, key_prev);
  grab_keysym(root, modmask_next_icon, key_next_icon);
  grab_keysym(root, modmask_prev_icon, key_prev_icon);
  grab_keysym(root, modmask_iconify, key_iconify);
  grab_keysym(root, modmask_close, key_close);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), bgc, &bg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), ibgc, &ibg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), fgc, &fg, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, screen), ifgc, &ifg, &dummy);
  font = XLoadQueryFont(dpy, fontc);
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
  icon_width = (XDisplayWidth(dpy, screen) - (hmaxicons - 1)) / hmaxicons;
}

