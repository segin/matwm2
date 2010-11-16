// client.c
extern client **clients, *current;
extern int cn;
void add_client(Window w);
void remove_client(client *c, int fc);
void draw_client(client *c);
void draw_icon(client *c);
void alloc_clients(void);
void move(client *c, int x, int y);
void resize(client *c, int width, int height);
void focus(client *c);
void raise_client(client *c);
void lower_client(client *c);
void maximise(client *c);
void set_shape(client *c);
int client_number(client *c);
client *owner(Window w);

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
int snapx(client *c, int nx, int ny);
int snapy(client *c, int nx, int ny);
int snaph(client *c, int nx, int ny);
int snapv(client *c, int nx, int ny);

// events.c
extern int (*evh)();
void handle_event(XEvent ev);

// icons.c
Bool isunmap(Display *display, XEvent *event, XPointer arg);
void iconify(client *c);
void restore(client *c);

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

// x11.c
int xerrorhandler(Display *display, XErrorEvent *xerror);
void getnormalhints(client *c);
int getstatehint(Window w);
int get_wm_state(Window w);
int get_wm_transient_for(Window w, Window *ret);
void set_wm_state(Window w, long state);
void get_mwm_hints(client *c);
void configurenotify(client *c);
int has_protocol(Window w, Atom protocol);
void delete_window(client *c);
int gxo(client *c, int i);
int gyo(client *c, int i);

// xev.c
char *event_name(XEvent ev);

