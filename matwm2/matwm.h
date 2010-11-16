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
  int           x, y, width, height, flags, layer, xo, yo, oldbw;
  int           expand_x, expand_y, expand_width, expand_height;
  XSizeHints    normal_hints;
  char          *name;
} client;

#define ICONIC          (1 << 0)
#define MAXIMISED       (1 << 1)
#define EXPANDED        (1 << 2)
#define FULLSCREEN      (1 << 3)
#define SHAPED          (1 << 4)
#define HAS_TITLE       (1 << 5)
#define HAS_BORDER      (1 << 6)
#define HAS_BUTTONS     (1 << 7)
#define CAN_MOVE        (1 << 8)
#define CAN_RESIZE      (1 << 9)
#define NO_STRUT        (1 << 10)

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
  KA_FULLSCREEN,
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

#define MOVE    0
#define RESIZE  1

#include "mwm_hints.h"
#include "ewmh.h"
#include "defaults.h"
#include "all.h"

