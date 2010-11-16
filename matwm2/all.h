// functions from actions.c
int client_move(client *c, int x, int y);
int client_resize(client *c, int width, int height);
void client_focus(client *c);
void client_raise(client *c);
void client_lower(client *c);
void client_set_layer(client *c, int layer);
void client_toggle_state(client *c, int state);
void client_expand(client *c, int d);
void client_toggle_title(client *c);
void client_iconify(client *c);
void client_restore(client *c);
void client_save(client *c);
void client_to_border(client *c, char *a);

// global variables from buttons.c
extern Window button_current;
extern int button_down;

// functions from buttons.c
void buttons_create(client *c);
void buttons_draw(client *c);
void button_draw(client *c, Window b);
void buttons_update(client *c);
int button_handle_event(XEvent ev);

// global variables from client.c
extern client **clients, **stacking, *current;
extern int cn, nicons;

// functions from client.c
void client_add(Window w);
void client_show(client *c);
void client_hide(client *c);
void client_deparent(client *c);
void client_remove(client *c);
void client_grab_button(client *c, int button);
void client_grab_buttons(client *c);
void client_draw_title(client *c);
void client_update_name(client *c);
void client_set_bg(client *c, XColor color, XColor border);
void clients_apply_stacking(void);
void client_update_pos(client *c);
void client_update_size(client *c);
void client_update(client *c);
void client_update_title(client *c);
void client_warp(client *c);
void clients_alloc(void);

// global variables from config.c
extern XColor bg, ibg, fg, ifg;
extern GC gc, igc, bgc, ibgc;
extern int border_width, text_height, title_height, button_parent_width, snapat, button1, button2, button3, button4, button5, dc, click_focus, click_raise;
extern XFontStruct *font;
extern char *no_title;

// functions from config.c
void cfg_read(void);
void cfg_parse(char *cfg);
void cfg_set_opt(char *key, char *value);
void str_bool(char *str, int *b);
KeySym str_key(char **str, unsigned int *mask);
unsigned int str_modifier(char *name);
int str_buttonaction(char *str);
int str_keyaction(char *str);

// global variables from drag.c
extern int drag_mode, drag_button, drag_xo, drag_yo;

// functions from drag.c
void drag_start(int mode, int button, int x, int y);
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

// global variables from ewmh.c
extern Atom ewmh_atoms[EWMH_ATOM_COUNT];
extern long ewmh_strut[4];

// functions from ewmh.c
void ewmh_initialize(void);
int ewmh_handle_event(XEvent ev);
int get_ewmh_hints(client *c);
void ewmh_update_extents(client *c);
void ewmh_update_geometry(void);
void ewmh_update_desktop(client *c);
void ewmh_set_desktop(int d);
void ewmh_set_active(client *c);
void ewmh_update_allowed_actions(client *c);
void ewmh_update_state(client *c);
void ewmh_update_stacking(void);
void ewmh_update_clist(void);
void ewmh_update_strut(void);

// functions from info.c
int client_x(client *c);
int client_y(client *c);
int client_width(client *c);
int client_height(client *c);
int client_border(client *c);
int client_border_intern(client *c);
int client_title(client *c);
int client_width_total(client *c);
int client_height_total(client *c);
int client_width_total_intern(client *c);
int client_height_total_intern(client *c);
int title_width(client *c);
int client_number(client **array, client *c);
client *owner(Window w);

// global variables from input.c
extern unsigned int mousemodmask, *mod_ignore;
extern XModifierKeymap *modmap;
extern keybind *keys;
extern int keyn, nmod_ignore;

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
void button_ungrab(Window w, unsigned int button, unsigned int modmask);
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
void quit(void);
void error(void);
void qsh(int sig);

// functions from misc.c
void spawn(char *cmd);
int read_file(char *path, char **buf);
char *eat(char **str, char *until);

// global variables from vdesk.c
extern int desktop;

// functions from vdesk.c
void desktop_goto(int d);
void client_to_desktop(client *c, int d);

// global variables from wlist.c
extern Window wlist;
extern int wlist_width;

// functions from wlist.c
void wlist_start(XEvent ev);
void wlist_end(int err);
client *wlist_next(void);
client *wlist_prev(void);
int wlist_handle_event(XEvent ev);
int wlist_update(void);
void wlist_item_draw(client *c);

// functions from x11.c
int xerrorhandler(Display *display, XErrorEvent *xerror);
void get_normal_hints(client *c);
int get_state_hint(Window w);
int get_wm_state(Window w);
void set_wm_state(Window w, long state);
void get_mwm_hints(client *c);
void set_shape(client *c);
int has_protocol(Window w, Atom protocol);
void delete_window(client *c);
int gxo(client *c, int initial);
int gyo(client *c, int initial);
int has_child(Window parent, Window child);
int isviewable(Window w);
Bool isunmap(Display *display, XEvent *event, XPointer arg);

