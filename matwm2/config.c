#include "matwm.h"

XColor bg, ibg, fg, ifg;
GC gc, igc;
int border_width, title_height, snapat, button1, button2, button3, button4, button5;
XFontStruct *font;
char *no_title = NO_TITLE;

void cfg_read(void) {
  char *home = getenv("HOME");
  int cfglen = home ? strlen(home) + strlen(CFGFN) + 2 : 0;
  char *cfg, *cfgfn = (char *) malloc(cfglen);
  XGCValues gv;
  cfg = (char *) malloc(strlen(DEF_CFG));
  strncpy(cfg, DEF_CFG, strlen(DEF_CFG));
  cfg_parse(cfg);
  free((void *) cfg);
  if(home) {
    strncpy(cfgfn, home, cfglen);
    strncat(cfgfn, "/", cfglen);
    strncat(cfgfn, CFGFN, cfglen);
    if(read_file(cfgfn, &cfg) > 0) {
      cfg_parse(cfg);
      free((void *) cfg);
    }
  }
  free((void *) cfgfn);
  update_keys();
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

void cfg_parse(char *cfg) {
  char *opt, *key, *val;
  keybind *old_keys = keys;
  while(cfg) {
    opt = eat(&cfg, "\n");
    key = eat(&opt, ":");
    if(opt == NULL)
      continue;
    while(*opt == ' ' || *opt == '\t')
      opt++;
    val = opt;
    if(strcmp(key, "key") == 0) {
      if(old_keys) {
        free_keys();
        old_keys = NULL;
      }
      bind_key(val);
    } else cfg_set_opt(key, val);
  }
}

void cfg_set_opt(char *key, char *value) {
  XColor dummy;
  int i;
  if(strcmp(key, "background") == 0)
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), value, &bg, &dummy);
  if(strcmp(key, "inactive_background") == 0)
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), value, &ibg, &dummy);
  if(strcmp(key, "foreground") == 0)
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), value, &fg, &dummy);
  if(strcmp(key, "inactive_foreground") == 0)
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), value, &ifg, &dummy);
  if(strcmp(key, "font") == 0) {
    font = XLoadQueryFont(dpy, value);
    if(!font) {
      fprintf(stderr, "error: font not found\n");
      exit(1);
    }
  }
  if(strcmp(key, "border_width") == 0)
    border_width = strtol(value, NULL, 0);
  if(strcmp(key, "snap") == 0)
    snapat = strtol(value, NULL, 0);
  if(strcmp(key, "button1") == 0)
    button1 = str_buttonaction(value);
  if(strcmp(key, "button2") == 0)
    button2 = str_buttonaction(value);
  if(strcmp(key, "button3") == 0)
    button3 = str_buttonaction(value);
  if(strcmp(key, "button4") == 0)
    button4 = str_buttonaction(value);
  if(strcmp(key, "button5") == 0)
    button5 = str_buttonaction(value);
  if(strcmp(key, "mouse_modifier") == 0)
    str_key(&value, &mousemodmask);
  if(strcmp(key, "ignore_modifier") == 0)
    while(value) {
      mod_ignore = (unsigned int *) realloc((void *) mod_ignore, (nmod_ignore + nmod_ignore + 2) * sizeof(unsigned int));
      if(!mod_ignore)
        error();
      mod_ignore[nmod_ignore] = str_modifier(eat(&value, " \t"));
      for(i = 0; i < nmod_ignore; i++)
        mod_ignore[nmod_ignore + 1 + i] = mod_ignore[i] | mod_ignore[nmod_ignore];
      nmod_ignore += nmod_ignore + 1;
    }
}

KeySym str_key(char **str, unsigned int *mask) {
  int mod;
  char *k;
  *mask = 0;
  while(*str) {
    k = eat(str, "\t ");
    mod = str_modifier(k);
    if(!mod)
      return XStringToKeysym(k);
    *mask = *mask | mod;
  }
  return 0;
}

unsigned int str_modifier(char *name) {
  if(strcmp(name, "shift") == 0)
    return ShiftMask;
  if(strcmp(name, "lock") == 0)
    return LockMask;
  if(strcmp(name, "control") == 0)
    return ControlMask;
  if(strcmp(name, "mod1") == 0)
    return Mod1Mask;
  if(strcmp(name, "mod2") == 0)
    return Mod2Mask;
  if(strcmp(name, "mod3") == 0)
    return Mod3Mask;
  if(strcmp(name, "mod4") == 0)
    return Mod4Mask;
  if(strcmp(name, "mod5") == 0)
    return Mod5Mask;
  return 0;
}

int str_buttonaction(char *str) {
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

int str_keyaction(char *str) {
  if(strcmp(str, "next") == 0)
    return KA_NEXT;
  if(strcmp(str, "prev") == 0)
    return KA_PREV;
  if(strcmp(str, "iconify") == 0)
    return KA_ICONIFY;
  if(strcmp(str, "maximise") == 0)
    return KA_MAXIMISE;
  if(strcmp(str, "topleft") == 0)
    return KA_TOPLEFT;
  if(strcmp(str, "topright") == 0)
    return KA_TOPRIGHT;
  if(strcmp(str, "bottomleft") == 0)
    return KA_BOTTOMLEFT;
  if(strcmp(str, "bottomright") == 0)
    return KA_BOTTOMRIGHT;
  if(strcmp(str, "close") == 0)
    return KA_CLOSE;
  if(strcmp(str, "exec") == 0)
    return KA_EXEC;
  return KA_NONE;
}

