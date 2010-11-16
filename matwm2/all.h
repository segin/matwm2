// functions from actions.c
int client_move(client *c, int x, int y);
int client_resize(client *c, int width, int height);
void client_focus(client *c);
void client_raise(client *c);
void client_lower(client *c);
void client_maximise(client *c);
void client_expand(client *c);
void client_toggle_title(client *c);
void client_iconify(client *c);
void client_restore(client *c);

// global variables from buttons.c
extern Window button_current;
extern int button_down;

// functions from buttons.c
void buttons_create(client *c);
void buttons_draw(client *c);
void button_draw(client *c, Window b);
void buttons_update(client *c);
int handle_button_event(XEvent ev);

// global variables from client.c
extern client **clients, *current;
extern int cn;

// functions from client.c
void client_add(Window w);
void client_deparent(client *c);
void client_remove(client *c);
void client_draw_border(client *c);
void client_draw_title(client *c);
void clients_alloc(void);
void client_set_bg(client *c, XColor color);
int has_window(client *c);
int client_number(client *c);
client *owner(Window w);

// global variables from config.c
extern XColor bg, ibg, fg, ifg;
extern GC gc, igc;
extern int border_width, text_height, title_height, button_parent_width, snapat, button1, button2, button3, button4, button5;
extern XFontStruct *font;
extern char *no_title;

// functions from config.c
void cfg_read(void);
void cfg_parse(char *cfg);
void cfg_set_opt(char *key, char *value);
KeySym str_key(char **str, unsigned int *mask);
unsigned int str_modifier(char *name);
int str_buttonaction(char *str);
int str_keyaction(char *str);

// global variables from drag.c
extern XButtonEvent be;
extern int xo, yo;

// functions from drag.c
void drag_start(XEvent ev);
void drag_end(void);
int drag_handle_event(XEvent ev);
int drag_release_wait(XEvent ev);
int snapx(client *c, int nx, int ny);
int snapy(client *c, int nx, int ny);
int snaph(client *c, int nx, int ny);
int snapv(client *c, int nx, int ny);

// global variables from events.c
extern int (*evh)(XEvent);

// functions from events.c
void handle_event(XEvent ev);

// functions from evn.c
char *event_name(XEvent ev);

// global variables from input.c
extern unsigned int mousemodmask, *mod_ignore;
extern XModifierKeymap *modmap;
extern keybind *keys;
extern int keyn, nmod_ignore ;

// functions from input.c
void key_bind(char *str);
void keys_update(void);
void keys_ungrab(void);
void keys_free(void);
void key_grab(keybind key);
void key_ungrab(keybind key);
int buttonaction(int button);
int keyaction(XEvent ev);
char *keyarg(XEvent ev);
int key_to_mask(KeyCode key);
void button_grab(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask);
int cmpmodmask(int m1, int m2);

// global variables from main.c
extern Display *dpy;
extern int screen, display_width, display_height, have_shape, shape_event, qsfd[2];
extern Window root;
extern Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints;
extern XSetWindowAttributes p_attr;
extern char *dn;

// functions from main.c
int main(int argc, char *argv[]);
void end(void);
void error(void);
void qsh(int sig);

// functions from misc.c
void spawn(char *cmd);
int read_file(char *path, char **buf);
char *eat(char **str, char *until);

// global variables from wlist.c
extern Window wlist;
extern int wlist_width;

// functions from wlist.c
void wlist_start(XEvent ev);
void wlist_end(void);
int wlist_handle_event(XEvent ev);
void wlist_update(void);
void wlist_item_draw(client *c);

// functions from x11.c
int xerrorhandler(Display *display, XErrorEvent *xerror);
void get_normal_hints(client *c);
int get_state_hint(Window w);
int get_wm_state(Window w);
int get_wm_transient_for(Window w, Window *ret);
void set_wm_state(Window w, long state);
void get_mwm_hints(client *c);
void set_shape(client *c);
int has_protocol(Window w, Atom protocol);
void delete_window(client *c);
int gxo(client *c, int initial);
int gyo(client *c, int initial);
Bool isunmap(Display *display, XEvent *event, XPointer arg);

