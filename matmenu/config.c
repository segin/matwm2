#include "matmenu.h"

XColor bg, ibg, fg, ifg;
GC gc, igc;
int text_height, item_height, ncw = -1, menu_width = -1, menu_height;
XFontStruct *font;

void cfg_read(void) {
  char *home = getenv("HOME");
  char *cfg, *cfgfn;
  XGCValues gv;
  cfg = (char *) malloc(strlen(DEF_CFG) + 1);
  strncpy(cfg, DEF_CFG, strlen(DEF_CFG));
  cfg_parse(cfg);
  free((void *) cfg);
  if(home) {
    cfgfn = (char *) malloc(strlen(home) + strlen(CFGFN) + 2);
    strncpy(cfgfn, home, strlen(home) + 1);
    strncat(cfgfn, "/", 1);
    strncat(cfgfn, CFGFN, strlen(CFGFN));
    if(read_file(cfgfn, &cfg) > 0) {
      cfg_parse(cfg);
      free((void *) cfg);
    }
    free((void *) cfgfn);
  }
  keys_update();
  text_height = font->max_bounds.ascent + font->max_bounds.descent;
  item_height = text_height + 6;
  menu_height = ((item_height + 1) * (ncw + 1)) + 1;
  gv.function = GXinvert;
  gv.subwindow_mode = IncludeInferiors;
  gv.line_width = 1;
  gv.font = font->fid;
  gv.function = GXcopy;
  gv.foreground = fg.pixel;
  gc = XCreateGC(dpy, root, GCFunction | GCSubwindowMode | GCLineWidth | GCForeground | GCFont, &gv);
  gv.foreground = ifg.pixel;
  igc = XCreateGC(dpy, root, GCFunction | GCSubwindowMode | GCLineWidth | GCForeground | GCFont, &gv);
	c = ncw;
}

void cfg_parse(char *cfg) {
  char *opt, *key, *val;
  keybind *old_keys = keys;
  while(cfg) {
    opt = eat(&cfg, "\n");
    opt = eat(&opt, "#");
    key = eat(&opt, "\t ");
    if(opt == NULL)
      continue;
    while(*opt == ' ' || *opt == '\t')
      opt++;
    val = opt;
    if(strcmp(key, "key") == 0) {
      if(old_keys) {
        keys_free();
        old_keys = NULL;
      }
      key_bind(val);
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
  if(strcmp(key, "width") == 0 && menu_width == -1)
    menu_width = strtol(value, NULL, 0);
  if(strcmp(key, "height") == 0 && ncw == -1)
    ncw = strtol(value, NULL, 0);
  if(strcmp(key, "exec") == 0)
    spawn(value);
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

int str_keyaction(char *str) {
  if(strcmp(str, "del") == 0)
    return KA_DEL;
  if(strcmp(str, "ok") == 0)
    return KA_OK;
  if(strcmp(str, "cancel") == 0)
    return KA_CANCEL;
  if(strcmp(str, "complete") == 0)
    return KA_COMPLETE;
  if(strcmp(str, "next") == 0)
    return KA_NEXT;
  if(strcmp(str, "prev") == 0)
    return KA_PREV;
  return KA_NONE;
}

