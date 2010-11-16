// .matwmrc

// client.c
extern client *clients;
extern int cn, current;
void add_client(Window w);
void remove_client(int n, int fc);
void draw_client(int n);
void alloc_clients(void);
void move(int n, int x, int y);
void resize(int n, int width, int height);
void focus(int n);
void next(int iconic, int warp);
void prev(int iconic, int warp);
void restack_client(int n, int top);
void maximise(int n);

// config.c
extern XColor bg, ibg, fg, ifg;
extern int border_width, title_height, hmaxicons, icon_width;
extern char *cbutton1, *cbutton2, *cbutton3, *cbutton4, *cbutton5, *ibutton1, *ibutton2, *ibutton3, *ibutton4, *ibutton5;
extern key key_next, key_prev, key_next_icon, key_prev_icon, key_iconify, key_maximise, key_close, key_bottomleft, key_bottomright, key_topleft, key_topright;
extern XFontStruct *font;
extern GC gc, igc;
extern XrmDatabase cfg;
char *xrm_getstr(XrmDatabase db, char *opt_name, char *def);
key xrm_getkey(XrmDatabase db, char *opt_name, char *def);
int xrm_getint(XrmDatabase db, char *opt_name, int def);
char *buttonaction(int n, int button);
KeyCode string_to_key(char *str, int *mask);
void config_read(void);

// events.c
void handle_event(XEvent ev);

// icons.c
extern int icons_ontop;
void sort_icons(void);
void restack_icons(int top);
void iconify(int n);
void restore(int n);

// input.c
extern unsigned int mousemodmask, numlockmask;
extern XModifierKeymap *modmap;
void grab_key(Window w, unsigned int modmask, KeyCode key);
void grab_button(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask);
void drag(int n, XButtonEvent *be, int res);
int getmodifier(KeyCode key);
void mapkeys(void);

// main.c
extern Display *dpy;
extern int screen, display_width, display_height;
extern Window root;
extern Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state;
extern XSetWindowAttributes p_attr;
void open_display(char *display);
void end(void);
void quit(int sig);
int main(int argc, char *argv[]);

// x11.c
int xerrorhandler(Display *display, XErrorEvent *xerror);
void getnormalhints(int n);
int getstatehint(Window w);
int get_wm_state(Window w);
void set_wm_state(Window w, long state);
void configurenotify(int n);
int has_protocol(Window w, Atom protocol);
void delete_window(int n);
int gxo(int c, int i);
int gyo(int c, int i);

