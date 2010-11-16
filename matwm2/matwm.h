#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>

#define MINSIZE 15

#define DEBUG

// client structure
typedef struct client {
  Window window, parent;
  int x, y, width, height, oldbw;
  XSizeHints normal_hints;
  char *name;
} client;

// client.c
extern client *clients;
extern int cn, current;
void add_client(Window w, int g);
void remove_client(int n);
void draw_client(int n);
void add_initial_clients(void);
void alloc_clients(void);
int gxo(int c, int i);
int gyo(int c, int i);
void getnormalhints(int n);
void configurenotify(int n);
int has_protocol(Window w, Atom protocol);
void set_wm_state(Window w, long state);
void configure(int c, XConfigureRequestEvent *e);
void delete_window(int n);
void move(int n, int x, int y);
void resize(int n, int width, int height);
void focus(int n);
void next(int warp);
void prev(int warp);

// config.c
extern XColor bg, ibg;
extern int border_width, title_height;
extern unsigned int mousemodmask, move_button, resize_button, lower_button;
extern XFontStruct *font;
extern GC gc, igc;
void config_read(void);

// error.c
int xerrorhandler(Display *display, XErrorEvent *xerror);

// events.c
void handle_event(XEvent ev);

// input.c
void grab_keysym(Window w, int modmask, KeySym key);
void grab_button(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask);
void drag(int n, XButtonEvent *be);

// main.c
extern Display *dpy;
extern int screen;
extern Window root;
extern unsigned int numlockmask;
extern Atom wm_protocols, wm_delete, wm_state;
void open_display(char *display);
void end(void);
void quit(int sig);
int main(int argc, char *argv[]);

