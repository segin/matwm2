// global variables from config.c
extern XColor bg, ibg, fg, ifg;
extern GC gc, igc;
extern int text_height, item_height, ncw, menu_width, menu_height;
extern XFontStruct *font;

// functions from config.c
void cfg_read(void);
void cfg_parse(char *cfg);
void cfg_set_opt(char *key, char *value);
KeySym str_key(char **str, unsigned int *mask);
unsigned int str_modifier(char *name);
int str_keyaction(char *str);

// global variables from events.c
extern int current, p, scroll, c;
extern char buf[BUF_SIZE], *cc;

// functions from events.c
void complete(void);
void handle_event(XEvent ev);

// global variables from input.c
extern unsigned int *mod_ignore;
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
int keyaction(XEvent ev);
char *keyarg(XEvent ev);
int key_to_mask(KeyCode key);
int cmpmodmask(int m1, int m2);

// global variables from main.c
extern Display *dpy;
extern int screen, display_width, display_height, qsfd[2], ncomp;
extern Window root, menu, input, *cw;
extern Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state;
extern XSetWindowAttributes p_attr;
extern char *dn, **comp;

// functions from main.c
int main(int argc, char *argv[]);
void end(void);
void error(void);
void qsh(int sig);

// functions from misc.c
void spawn(char *cmd);
int read_file(char *path, char **buf);
void readcomp(int fd);
char *eat(char **str, char *until);

// functions from x11.c
int xerrorhandler(Display *display, XErrorEvent *xerror);

