#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/Xresource.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct client {
  Window window, parent;
  int x, y, width, height, oldbw, iconic, maximised, prev_x, prev_y, prev_width, prev_height;
  XSizeHints normal_hints;
  char *name;
} client;

typedef struct key {
  KeyCode code;
  int mask;
} key;

#include "defaults.h"
#include "all.h"

