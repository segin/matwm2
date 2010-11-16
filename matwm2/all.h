// client.c
extern client *clients;
extern int cn, current;
void add_client(Window w);
void remove_client(int n, int fc);
void draw_client(int n);
void draw_icon(int n);
void alloc_clients(void);
void move(int n, int x, int y);
void resize(int n, int width, int height);
void focus(int n);
void restack_client(int n, int top);
void maximise(int n);
void set_shape(int c);

// config.c
extern XColor bg, ibg, fg, ifg;
extern int border_width, title_height, snapat;
extern int button1, button2, button3, button4, button5;
extern key key_next, key_prev, key_iconify, key_maximise, key_close, key_bottomleft, key_bottomright, key_topleft, key_topright;
extern XFontStruct *font;
extern GC gc, igc;
extern XrmDatabase cfg;
char *xrm_getstr(XrmDatabase db, char *opt_name, char *def);
key xrm_getkey(XrmDatabase db, char *opt_name, char *def);
int xrm_getint(XrmDatabase db, char *opt_name, int def);
int strbuttonaction(char *str);
int buttonaction(int button);
KeyCode string_to_key(char *str, int *mask);
void config_read(void);

// drag.c
extern XButtonEvent be;
extern int xo, yo;
Bool isrelease(Display *display, XEvent *event, XPointer arg);
void drag_start(XEvent ev);
void drag_end(void);
int drag_handle_event(XEvent ev);
int drag_release_wait(XEvent ev);
int snapx(int n, int nx, int ny);
int snapy(int n, int nx, int ny);
int snaph(int n, int nx, int ny);
int snapv(int n, int nx, int ny);

// events.c
extern int (*evh)();
void handle_event(XEvent ev);

// icons.c
Bool isunmap(Display *display, XEvent *event, XPointer arg);
void iconify(int n);
void restore(int n);

// input.c
extern unsigned int mousemodmask, numlockmask;
extern XModifierKeymap *modmap;
void grab_key(Window w, unsigned int modmask, KeyCode key);
void grab_button(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask);
int getmodifier(char *name);
void mapkeys(void);
int key_to_mask(KeyCode key);

// main.c
extern Display *dpy;
extern int screen, display_width, display_height, have_shape, shape_event, qsfd[2];
extern Window root;
extern Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints;
extern XSetWindowAttributes p_attr;
void error(void);
void end(void);
void qsh(int sig);
int main(int argc, char *argv[]);

// wlist.c
extern Window wlist;
extern int wlist_width;
void wlist_start(XEvent ev);
void wlist_end(void);
int wlist_handle_event(XEvent ev);
void wlist_update(void);
void wlist_draw(void);

// x11.c
int xerrorhandler(Display *display, XErrorEvent *xerror);
void getnormalhints(int n);
int getstatehint(Window w);
int get_wm_state(Window w);
int get_wm_transient_for(Window w, Window *ret);
void set_wm_state(Window w, long state);
void get_mwm_hints(int n);
void configurenotify(int n);
int has_protocol(Window w, Atom protocol);
void delete_window(int n);
int gxo(int c, int i);
int gyo(int c, int i);

