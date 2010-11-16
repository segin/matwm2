#ifndef __MATWM_H__
#define __MATWM_H__
#include <X11/Xlib.h>
#include <X11/Xutil.h> /* for XSizeHints structure and XGetNormalHints() */
#include <X11/Xatom.h> /* for Atom datatype and XInetnAtom() etc */
#ifdef USE_SHAPE
#include <X11/extensions/shape.h>
#endif
#ifdef USE_XFT
#include <X11/Xft/Xft.h>
#endif
#include <stdlib.h> /* for malloc() and perhaps some other stuff */
#include <stdio.h> /* for printf(), fprintf(), perror(), etc */
#include <string.h> /* for strlen(), strcmp(), strncmp() and strcat() */

#ifndef HAVE_VFORK
#define vfork fork
#endif

#define NAME "matwm2" /* our name, schould we forget it */
#define CFGFN ".matwmrc" /* configuration file in $HOME */
#define GCFGFN "/etc/matwmrc" /* global configuration file */
#define MINSIZE 5 /* minimum size of windows (for both x and y) */
#define NO_TITLE "-" /* windows that have no title get this name */
#define INSANE_CONFIG_FILE_SIZE (1024 * 1024) /* if a config file is bigger then this, we won't be surprised and quit if it can't be loaded to memory */

typedef struct {
	Window w;
	unsigned char action;
} button;

typedef struct {
	Window      window, parent, title, wlist_item, button_parent_right, button_parent_left;
	int         x, y, width, height, flags, layer, desktop, xo, yo, oldbw, screen;
	int         expand_x, expand_y, expand_width, expand_height, title_width, buttons_left_width, buttons_right_width, nbuttons;
	Pixmap      title_pixmap;
#ifdef USE_XFT
	XftDraw     *title_draw, *wlist_draw;
#endif
	XSizeHints  normal_hints;
	char        *name;
	button      *buttons;
} client;

#define ICONIC          (1 << 0) /* these bits are for the "flags" member of above structure */
#define MAXIMIZED_L     (1 << 1)
#define MAXIMIZED_R     (1 << 2)
#define MAXIMIZED_T     (1 << 3)
#define MAXIMIZED_B     (1 << 4)
#define EXPANDED_L      (1 << 5)
#define EXPANDED_R      (1 << 6)
#define EXPANDED_T      (1 << 7)
#define EXPANDED_B      (1 << 8)
#define FULLSCREEN      (1 << 9)
#define SHAPED          (1 << 10)
#define HAS_TITLE       (1 << 11)
#define HAS_BORDER      (1 << 12)
#define HAS_BUTTONS     (1 << 13)
#define CAN_MOVE        (1 << 14)
#define CAN_RESIZE      (1 << 15)
#define NO_STRUT        (1 << 16)
#define DONT_LIST       (1 << 17)
#define RESTORE         (1 << 18)
#define DONT_FOCUS      (1 << 19)
#define CLICK_FOCUS     (1 << 20)
#define DESKTOP_LOCKED  (1 << 21)
#define IS_TASKBAR      (1 << 22)

#define STICKY          -1 /* special virtual desktop that is metaphoric for any virtual desktop */

enum layers {
	TOPMOST,
	TOP,
	NORMAL,
	BOTTOM,
	DESKTOP,
	NLAYERS
};

typedef struct {
	unsigned char code;
	char *arg;
} action;

typedef struct {
	KeySym sym;
	KeyCode code;
	unsigned int mask;
	action *a;
} keybind;

enum { /* action codes */
	A_NONE,
	A_NEXT,
	A_PREV,
	A_ICONIFY,
	A_ICONIFY_ALL,
	A_MAXIMIZE,
	A_FULLSCREEN,
	A_EXPAND,
	A_CLOSE,
	A_TITLE,
	A_TO_BORDER,
	A_EXEC,
	A_NEXT_DESKTOP,
	A_PREV_DESKTOP,
	A_STICKY,
	A_ONTOP,
	A_BELOW,
	A_RAISE,
	A_LOWER,
	A_MOVE,
	A_RESIZE
};

enum { /* for frame buttons */
	B_NONE,
	B_CLOSE,
	B_MAXIMIZE,
	B_EXPAND,
	B_ICONIFY,
	B_STICKY,
	B_ONTOP,
	B_BELOW
};

enum { /* fullscreen_stacking modes */
	FS_NORMAL,
	FS_ONTOP,
	FS_ALWAYS_ONTOP
};

enum { /* for sending to qsfd pipe */
	QUIT,
	ERROR,
	REINIT
};

typedef char bool;
enum { false, true };

#include "mwm_hints.h"
#include "ewmh.h"
#include "screens.h"
#include "defcfg.h"
#include "all.h"
#endif /* __MATWM_H__ */
