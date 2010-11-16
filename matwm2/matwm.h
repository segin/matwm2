#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/Xresource.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_DECOR_BORDER (1L << 1)
#define MWM_DECOR_TITLE (1L << 3)
#define border(c) (clients[c].border ? border_width : 0)
#define title(c) ((clients[c].title && clients[c].border) ? title_height : 0)
#define warp() XWarpPointer(dpy, None, clients[current].parent, 0, 0, 0, 0, clients[current].iconic ? icon_width - 1 : clients[current].width + (clients[current].border ? border_width : -1), clients[current].iconic ? 3 + title_height : ((clients[current].height + (clients[current].border ? border_width : -1)) + title(current)));

typedef struct {
  Window window, parent;
  int x, y, width, height, oldbw, iconic, maximised, prev_x, prev_y, prev_width, prev_height, title, border;
  XSizeHints normal_hints;
  char *name;
} client;

typedef struct {
  KeyCode code;
  int mask;
} key;

typedef struct {
  uint32_t  flags;
  uint32_t  functions;
  uint32_t  decorations;
  int32_t   input_mode;
  uint32_t  status;
} MWMHints;

#include "defaults.h"
#include "all.h"

