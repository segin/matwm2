/* global variables from main.c */
extern Display *dpy;
extern int screen, depth, have_shape, shape_event, qsfd[2];
extern Window root;
extern Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints;
extern XSetWindowAttributes p_attr;
extern char *dn, *perror_str;
extern Colormap colormap;
extern Visual *visual;

/* functions from main.c */
int main(int argc, char *argv[]);
void quit(void);
void qsfd_send(char s);
void sighandler(int sig);

/* global variables from wlist.c */
extern Window wlist;
extern int wlist_width, wlist_screen;
extern client *client_before_wlist;

/* functions from wlist.c */
void wlist_start(XEvent *ev);
void wlist_end(int err);
client *wlist_next(void);
client *wlist_prev(void);
bool wlist_handle_event(XEvent *ev);
int wlist_update(void);
void wlist_item_draw(client *c);

/* global variables from events.c */
extern bool (*evh)(XEvent *);
extern Time lastclick;
extern unsigned int lastbutton;
extern client *lastclick_client;

/* functions from events.c */
void handle_event(XEvent *ev);

/* global variables from config.c */
extern XColor bg, ibg, fg, ifg, bfg, ibfg;
extern GC gc, igc, bgc, ibgc;
extern int border_spacing, border_width, button_spacing, wlist_margin, wlist_maxwidth, wlist_item_height, text_height, title_height, button_size, title_spacing, snapat, dc, first, *buttons_right, nbuttons_right, *buttons_left, nbuttons_left, doubleclick_time, fullscreen_stacking, ewmh_screen;
extern bool center_title, center_wlist_items, click_focus, click_raise, focus_new, taskbar_ontop, map_center, drag_warp, allow_focus_stealing, correct_center, click_root;
extern action *button1, *button2, *button3, *button4, *button5, *double1, *double2, *double3, *double4, *double5;
extern action *root_button1, *root_button2, *root_button3, *root_button4, *root_button5, *root_double1, *root_double2, *root_double3, *root_double4, *root_double5;
#ifdef USE_XFT
extern XftFont *xftfont;
extern XftColor xftfg, xftbg, xftifg, xftibg;
#endif
extern XFontStruct *font;
extern char *no_title;

/* functions from config.c */
void cfg_read(int initial);
void cfg_parse_defaults(int initial);
void cfg_parse(char *cfg, int initial);
void cfg_set_opt(char *key, char *value, int initial);
void cfg_reinitialize(void);
void str_color(char *str, XColor *c);
#ifdef USE_XFT
void set_xft_color(XftColor *xftcolor, XColor xcolor);
#endif
void str_bool(char *str, bool *b);
void str_fsstacking(char *str, int *s);
KeySym str_key(char **str, unsigned int *mask);
unsigned int str_modifier(char *name);
void str_action(char *str, action **ret);
int str_wbutton(char *button);
void str_buttons(char *str, int **buttons, int *nbuttons);

/* global variables from client.c */
extern client **clients, **stacking, *current, *previous;
extern int cn, nicons;

/* functions from client.c */
void client_add(Window w, bool mapped);
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
void client_update_layer(client *c, int prev);
void client_clear_state(client *c);
void client_over_fullscreen(client *c);
void clients_alloc(void);

/* global variables from input.c */
extern unsigned int mousemodmask, nosnapmodmask, *mod_ignore;
extern XModifierKeymap *modmap;
extern keybind *keys;
extern int keyn, nmod_ignore;

/* functions from input.c */
void keys_alloc(int n);
void key_bind(char *str);
void keys_update(void);
void keys_ungrab(void);
void key_free(keybind *k);
void keys_free(void);
void key_grab(keybind key);
void key_ungrab(keybind key);
action *buttonaction(int button, int is_double);
action *root_buttonaction(int root_button, int is_double);
action *keyaction(XEvent *ev);
int key_to_mask(KeyCode key);
void button_grab(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask);
void button_ungrab(Window w, unsigned int button, unsigned int modmask);
int cmpmodmask(int m1, int m2);

/* global variables from x11.c */
extern int xerrorstatus;

/* functions from x11.c */
int xerrorhandler(Display *display, XErrorEvent *xerror);
bool select_root_events(void);
void get_normal_hints(client *c);
int get_state_hint(Window w);
int get_wm_state(Window w);
void set_wm_state(Window w, long state);
void get_mwm_hints(client *c);
#ifdef USE_SHAPE
void set_shape(client *c);
#endif
void configurenotify(client *c);
int has_protocol(Window w, Atom protocol);
void delete_window(client *c);
int gxo(client *c, bool initial);
int gyo(client *c, bool initial);
bool has_child(Window parent, Window child);
Window get_focus_window(void);
int isviewable(Window w);
Bool isunmap(Display *display, XEvent *event, XPointer arg);

/* global variables from drag.c */
extern int drag_xo, drag_yo, xr, yr;
extern unsigned int drag_button;
extern unsigned char drag_mode;

/* functions from drag.c */
void drag_start(unsigned char mode, int button, int x, int y);
void drag_end(void);
bool drag_handle_event(XEvent *ev);
bool drag_release_wait(XEvent *ev);
bool __snap(int x1, int x2, int *ret);
bool _snap(int r, int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, int *ret);
int snap(client *c, int nx, int ny, char axis);

/* functions from evn.c */
#ifdef DEBUG_EVENTS
char *event_name(XEvent *ev);
#endif /* DEBUG_EVENTS */

/* functions from misc.c */
void spawn(char *cmd);
int read_file(char *path, char **buf);
char *eat(char **str, char *until);
void unescape(char *str);
void *_malloc(size_t size);
void *_realloc(void *ptr, size_t size);
void error(void);

/* global variables from buttons.c */
extern button *button_current;
extern int button_down;

/* functions from buttons.c */
void buttons_create(client *c);
void buttons_draw(client *c);
void button_draw(client *c, button *b);
void buttons_update(client *c);
bool button_handle_event(XEvent *ev);

/* global variables from actions.c */
extern int all_iconic;

/* functions from actions.c */
void client_move(client *c, int x, int y);
void client_resize(client *c, int width, int height);
void client_focus(client *c, bool set_input_focus);
void client_raise(client *c);
void client_lower(client *c);
void client_fullscreen(client *c);
void client_set_layer(client *c, int layer);
void client_toggle_state(client *c, int state);
void client_expand_x(client *c, int d, int first);
void client_expand_y(client *c, int d, int first);
void client_expand(client *c, int d, bool a);
void client_toggle_title(client *c);
void client_iconify(client *c);
void client_restore(client *c);
void client_save(client *c);
void client_to_border(client *c, char *a);
void client_iconify_all(void);
void client_end_all_iconic(void);
void client_warp(client *c);
void client_focus_first(void);
void client_action(client *c, action *act, XEvent *ev);

/* global variables from ewmh.c */
extern Atom ewmh_atoms[EWMH_ATOM_COUNT];

/* functions from ewmh.c */
void ewmh_initialize(void);
void ewmh_update(void);
bool ewmh_handle_event(XEvent *ev);
void ewmh_get_hints(client *c);
void ewmh_update_extents(client *c);
void ewmh_update_geometry(void);
void ewmh_update_number_of_desktops(void);
void ewmh_update_desktop(client *c);
void ewmh_set_desktop(int d);
void ewmh_set_active(client *c);
void ewmh_update_allowed_actions(client *c);
void ewmh_update_state(client *c);
void ewmh_update_stacking(void);
void ewmh_update_clist(void);
void ewmh_update_strut(void);
void ewmh_update_showing_desktop(void);

/* global variables from vdesk.c */
extern int desktop;

/* functions from vdesk.c */
void desktop_goto(int d);
void client_to_desktop(client *c, int d);

/* functions from info.c */
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
int client_title_width(client *c);
int client_title_x(client *c);
bool client_visible(client *c);
int client_layer(client *c);
int client_number(client **array, client *c);
client *owner(Window w);

/* global variables from screens.c */
extern screen_dimensions *screens;
extern int nscreens, cs;

/* functions from screens.c */
bool screens_handle_event(XEvent *ev);
void screens_get(void);
void screens_update_current(void);
int intersect(int base_start, int base_len, int start, int len);
void client_update_screen(client *c);
int screens_leftmost(void);
int screens_rightmost(void);
int screens_topmost(void);
int screens_bottom(void);
bool screens_correct_center(int *x, int *y, int *width, int *height);

/* functions from opcodes.c */
#ifdef DEBUG
char *str_opcode(unsigned char opcode);
#endif /* DEBUG */

