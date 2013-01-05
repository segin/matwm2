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
  KeySym sym;
  KeyCode code;
  unsigned int mask, action;
  char *arg;
} keybind;

enum {KA_NONE, KA_DEL, KA_CANCEL, KA_OK, KA_COMPLETE, KA_NEXT, KA_PREV};

#define border(c)               ((!(c->flags & SHAPED) && c->flags & HAS_BORDER) ? border_width : 0)
#define title(c)                ((!(c->flags & SHAPED) && c->flags & HAS_TITLE && c->flags & HAS_BORDER) ? title_height : 0)
#define total_width(c)          (c->width + (border(c) * 2))
#define total_height(c)         (c->height + (border(c) * 2) + title(c))
#define warpto(c)               XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, c->width + (border(c) ? border_width : -1), (c->height + (border(c) ? border_width : -1)) + title(c));

#include "defaults.h"
#include "all.h"

