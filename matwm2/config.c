#include "matwm.h"

XColor bg, ibg, fg, ifg;
GC gc, igc;
int border_width, title_height, snapat, button1, button2, button3, button4, button5, keyn = 0;
XFontStruct *font;
keybind *keys = NULL;

void cfg_init(void) {
  XGCValues gv;
  char *home = getenv("HOME");
  int cfglen = home ? strlen(home) + strlen(CFGFN) + 2 : 0;
  char cfgfn[cfglen], *cfg;
  modmap = XGetModifierMapping(dpy);
  numlockmask = key_to_mask(XKeysymToKeycode(dpy, XK_Num_Lock));
  cfg = (char *) malloc(strlen(DEF_CFG));
  strncpy(cfg, DEF_CFG, strlen(DEF_CFG));
  readcfg(cfg);
  free((void *) cfg);
  if(home) {
    strncpy(cfgfn, home, cfglen);
    strncat(cfgfn, "/", cfglen);
    strncat(cfgfn, CFGFN, cfglen);
    if(read_file(cfgfn, &cfg) > 0) {
      readcfg(cfg);
      free((void *) cfg);
    }
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

void setopt(char *key, char *value) {
  XColor dummy;
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
    button1 = strbuttonaction(value);
  if(strcmp(key, "button2") == 0)
    button2 = strbuttonaction(value);
  if(strcmp(key, "button3") == 0)
    button3 = strbuttonaction(value);
  if(strcmp(key, "button4") == 0)
    button4 = strbuttonaction(value);
  if(strcmp(key, "button5") == 0)
    button5 = strbuttonaction(value);
  if(strcmp(key, "mouse_modifier") == 0)
    str_key(&value, &mousemodmask);
}

void readcfg(char *cfg) {
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
        unbind_keys();
        old_keys = NULL;
      }
      bind_key(val);
    } else setopt(key, val);
  }
}

void bind_key(char *str) {
  keybind k;
  k.code = str_key(&str, &k.mask);
  if(!str)
    return;
  k.action = strkeyaction(eat(&str, " \t"));
  if(str) {
    while(*str == ' ' || *str == '\t')
      str++;
    k.arg = (char *) malloc(strlen(str));
    strncpy(k.arg, str, strlen(str));
  } else k.arg = NULL;
  keys = (keybind *) realloc((void *) keys, (keyn + 1) * sizeof(keybind));
  if(!keys)
    error();
  keys[keyn] = k;
  keyn++;
  grab_key(root, k.mask, k.code);
}

void unbind_keys(void) {
  int i;
  for(i = 0; i < keyn; i++) {
    ungrab_key(root, keys[i].mask, keys[i].code);
    if(keys[i].arg)
      free((void *) keys[i].arg);
  }
  if(keys)
    free((void *) keys);
  keys = NULL;
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

int strkeyaction(char *str) {
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
  if(strcmp(str, "exec") == 0)
    return KA_EXEC;
  return KA_NONE;
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

KeyCode str_key(char **str, int *mask) {
  int mod;
  char *k;
  *mask = 0;
  while(*str) {
    k = eat(str, "\t ");
    mod = getmodifier(k);
    if(!mod)
      return XKeysymToKeycode(dpy, XStringToKeysym(k));
    *mask = *mask | mod;
  }
  return 0;
}

int read_file(char *path, char **buf) {
  struct stat sb;
  int r = 0, fd = open(path, O_RDONLY | O_EXLOCK);
  if(fd > 0) {
    if(fstat(fd, &sb) == 0) {
      *buf = (char *) malloc(sb.st_size);
      if(buf == NULL)
        error();
      r = read(fd, (void *) *buf, sb.st_size);
      if(r <= 0)
        free((void *) *buf);
    }
    close(fd);
  }
  return r;
}

char *eat(char **str, char *until) {
  char *ret = NULL, *c;
  while(1) {
    if(!ret && (**str != ' ' && **str != '\t'))
      ret = *str;
    if(**str == 0) {
      *str = NULL;
      return ret;
    }
    c = until;
    while(*c) {
      if(ret && (**str == *c || **str == 0)) {
        **str = 0;
        (*str)++;
        return ret;
      }
      c++;
    }
    (*str)++;
  }
}

