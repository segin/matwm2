#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/Xresource.h>
#include <X11/extensions/shape.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)
#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
#define MWM_DECOR_RESIZEH       (1L << 2)
#define MWM_DECOR_TITLE         (1L << 3)
#define MWM_DECOR_MENU          (1L << 4)
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)

#define BA_NONE         0
#define BA_MOVE         1
#define BA_RESIZE       2
#define BA_RAISE        3
#define BA_LOWER        4

#define border(c)       ((!c->shaped && c->border) ? border_width : 0)
#define title(c)        ((!c->shaped && c->title && c->border) ? title_height : 0)
#define total_width(c)  (c->width + (border(c) * 2))
#define total_height(c) (c->height + (border(c) * 2) + title(c))
#define warp()          XWarpPointer(dpy, None, current->parent, 0, 0, 0, 0, current->width + (border(current) ? border_width : -1), (current->height + (border(current) ? border_width : -1)) + title(current));
#define cmpmask(m1, m2) (m1 == m2 || m1 == (m2 | numlockmask) || m1 == (m2 | LockMask) || m1 == (m2 | LockMask | numlockmask))
#define iskey(k)        (cmpmask(ev.xkey.state, k.mask) && ev.xkey.keycode == k.code)
#define rmbit(m, b)     (m ^ (m & b))

typedef struct {
  Window        window, parent, icon;
  int           x, y, width, height, iconic, maximised;
  int           oldbw, prev_x, prev_y, prev_width, prev_height;
  int           title, border, resize, shaped;
  XSizeHints    normal_hints;
  char          *name;
} client;

typedef struct {
  KeyCode code;
  int mask;
} key;

typedef struct {
  unsigned long flags;
  unsigned long functions;
  unsigned long decorations;
  long          input_mode;
  unsigned long status;
} MWMHints;

#include "defaults.h"
#include "all.h"

