#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// client structure
typedef struct client {
  Window window, parent, taskbutton;
  int x, y, width, height, oldbw, minimised;
  XSizeHints normal_hints;
  char *name;
} client;

#include "defaults.h"
#include "all.h"

