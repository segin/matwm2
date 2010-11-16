#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/extensions/shape.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>

#define NAME "matwm2"

typedef struct {
  Window        window, parent, title, wlist_item, button_parent, button_iconify, button_maximise, button_expand, button_close;
  int           x, y, width, height, flags, layer;
  int           oldbw, prev_x, prev_y, prev_width, prev_height;
  int           expand_prev_x, expand_prev_y, expand_prev_width, expand_prev_height;
  XSizeHints    normal_hints;
  char          *name;
} client;

#define ICONIC          (1 << 0)
#define MAXIMISED       (1 << 1)
#define EXPANDED        (1 << 2)
#define SHAPED          (1 << 3)
#define HAS_TITLE       (1 << 4)
#define HAS_BORDER      (1 << 5)
#define HAS_BUTTONS     (1 << 6)
#define CAN_MOVE        (1 << 7)
#define CAN_RESIZE      (1 << 8)
#define NO_STRUT        (1 << 9)

enum layers {
  TOPMOST,
  TOP,
  NORMAL,
  BOTTOM,
  DESKTOP,
  NLAYERS
};

typedef struct {
  KeySym sym;
  KeyCode code;
  unsigned int mask, action;
  char *arg;
} keybind;

enum {
  KA_NONE,
  KA_NEXT,
  KA_PREV,
  KA_ICONIFY,
  KA_ICONIFY_ALL,
  KA_MAXIMISE,
  KA_EXPAND,
  KA_CLOSE,
  KA_TITLE,
  KA_TOPLEFT,
  KA_TOPRIGHT,
  KA_BOTTOMRIGHT,
  KA_BOTTOMLEFT,
  KA_EXEC
};

enum {
  BA_NONE,
  BA_MOVE,
  BA_RESIZE,
  BA_RAISE,
  BA_LOWER
};

#define border(c)               ((!(c->flags & SHAPED) && c->flags & HAS_BORDER) ? border_width : 0)
#define title(c)                ((!(c->flags & SHAPED) && c->flags & HAS_TITLE && c->flags & HAS_BORDER) ? title_height : 0)
#define total_width(c)          (c->width + (border(c) * 2))
#define total_height(c)         (c->height + (border(c) * 2) + title(c))
#define warpto(c)               XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, c->width + (border(c) ? border_width : -1), (c->height + (border(c) ? border_width : -1)) + title(c));

#include "mwm_hints.h"
#include "ewmh.h"
#include "defaults.h"
#include "all.h"

