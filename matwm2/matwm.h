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

typedef struct {
  Window        window, parent, wlist_item;
  int           x, y, width, height, iconic, maximised;
  int           oldbw, prev_x, prev_y, prev_width, prev_height;
  int           title, border, resize, shaped;
  XSizeHints    normal_hints;
  char          *name;
} client;

typedef struct {
  KeySym sym;
  KeyCode code;
  unsigned int mask, action;
  char *arg;
} keybind;

enum {KA_NONE, KA_NEXT, KA_PREV, KA_ICONIFY, KA_MAXIMISE, KA_CLOSE, KA_TOPLEFT, KA_TOPRIGHT, KA_BOTTOMRIGHT, KA_BOTTOMLEFT, KA_EXEC};
enum {BA_NONE, BA_MOVE, BA_RESIZE, BA_RAISE, BA_LOWER};

#define border(c)       ((!c->shaped && c->border) ? border_width : 0)
#define title(c)        ((!c->shaped && c->title && c->border) ? title_height : 0)
#define total_width(c)  (c->width + (border(c) * 2))
#define total_height(c) (c->height + (border(c) * 2) + title(c))
#define warp()          XWarpPointer(dpy, None, current->parent, 0, 0, 0, 0, current->width + (border(current) ? border_width : -1), (current->height + (border(current) ? border_width : -1)) + title(current));

#include "mwm_hints.h"
#include "defaults.h"
#include "all.h"

