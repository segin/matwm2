// buttons.c
extern Window button_current;
extern int button_down;
void buttons_create(client *c);
void buttons_draw(client *c);
int handle_button_event(XEvent ev);

// client.c
extern client **clients, *current;
extern int cn;
void add_client(Window w);
void remove_client(client *c);
void draw_client(client *c);
void alloc_clients(void);
int move(client *c, int x, int y);
int resize(client *c, int width, int height);
void focus(client *c);
void raise_client(client *c);
void lower_client(client *c);
void maximise(client *c);
void expand(client *c);
void toggle_title(client *c);
void set_shape(client *c);
int client_number(client *c);
client *owner(Window w);

// config.c
extern XColor bg, ibg, fg, ifg;
extern GC gc, igc;
extern int border_width, text_height, title_height, button_parent_width, snapat, button1, button2, button3, button4, button5;
extern XFontStruct *font;
extern char *no_title;
void cfg_read(void);
void cfg_parse(char *cfg);
void cfg_set_opt(char *key, char *value);
KeySym str_key(char **str, unsigned int *mask);
unsigned int str_modifier(char *name);
int str_buttonaction(char *str);
int str_keyaction(char *str);

// drag.c
extern XButtonEvent be;
extern int xo, yo;
void drag_start(XEvent ev);
void drag_end(void);
int drag_handle_event(XEvent ev);
int drag_release_wait(XEvent ev);
int snapx(client *c, int nx, int ny);
int snapy(client *c, int nx, int ny);
int snaph(client *c, int nx, int ny);
int snapv(client *c, int nx, int ny);

// events.c
extern int (*evh)(XEvent);
Bool isgone(Display *display, XEvent *event, XPointer arg);
void handle_event(XEvent ev);

// evn.c
char *event_name(XEvent ev);

// icons.c
Bool isunmap(Display *display, XEvent *event, XPointer arg);
void iconify(client *c);
void restore(client *c);

// input.c
extern unsigned int mousemodmask, *mod_ignore;
extern XModifierKeymap *modmap;
extern keybind *keys;
extern int keyn, nmod_ignore;
void bind_key(char *str);
void update_keys(void);
void ungrab_keys(void);
void free_keys(void);
int buttonaction(int button);
int keyaction(XEvent ev);
char *keyarg(XEvent ev);
int key_to_mask(KeyCode key);
void grab_key(keybind key);
void ungrab_key(keybind key);
void grab_button(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask);
int cmpmodmask(int m1, int m2);

// main.c
extern Display *dpy;
extern int screen, display_width, display_height, have_shape, shape_event, qsfd[2];
extern Window root;
extern Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints;
extern XSetWindowAttributes p_attr;
extern char *dn;
int main(int argc, char *argv[]);
void end(void);
void error(void);
void qsh(int sig);

// misc.c
void spawn(char *cmd);
int read_file(char *path, char **buf);
char *eat(char **str, char *until);

// wlist.c
extern Window wlist;
extern int wlist_width;
void wlist_start(XEvent ev);
void wlist_end(void);
int wlist_handle_event(XEvent ev);
void wlist_update(void);
void wlist_item_draw(client *c);

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
int gxo(client *c, int initial);
int gyo(client *c, int initial);

