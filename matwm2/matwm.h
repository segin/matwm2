#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>

#define MINSIZE 15

#define DEBUG

// client structure
typedef struct client {
  Window window, parent, taskbutton;
  int x, y, width, height, oldbw, minimised;
  XSizeHints normal_hints;
  char *name;
} client;

// client.c
extern client *clients;
extern int cn, current;
void add_client(Window w, int g);
void update_taskbar(void);
void remove_client(int n);
void draw_client(int n);
void draw_taskbutton(int n);
void add_initial_clients(void);
void alloc_clients(void);
int gxo(int c, int i);
int gyo(int c, int i);
void getnormalhints(int n);
int getstatehint(Window w);
void configurenotify(int n);
int has_protocol(Window w, Atom protocol);
int get_wm_state(Window w);
void set_wm_state(Window w, long state);
void configure(int c, XConfigureRequestEvent *e);
void delete_window(int n);
void move(int n, int x, int y);
void resize(int n, int width, int height);
void focus(int n);
void next(int minimised, int warp);
void prev(int minimised, int warp);
void minimise(int n);

// config.c
extern XColor bg, ibg, fg, ifg;
extern int border_width, title_height, taskbar_width, taskbutton_width;
extern unsigned int mousemodmask, move_button, resize_button, raise_button, lower_button, tb_raise_button, tb_lower_button, tb_restore_button;
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
extern int screen, taskbar_visible;
extern Window root, taskbar;
extern unsigned int numlockmask;
extern Atom xa_wm_protocols, xa_wm_delete, xa_wm_state;
extern XSetWindowAttributes p_attr;
void open_display(char *display);
void end(void);
void quit(int sig);
int main(int argc, char *argv[]);

