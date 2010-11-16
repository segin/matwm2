#include "matwm.h"
#include <signal.h>

Display *dpy;
int screen, taskbar_visible = 0;
Window root, taskbar;
unsigned int numlockmask = 0;
Atom xa_wm_protocols, xa_wm_delete, xa_wm_state;
XSetWindowAttributes p_attr;

void open_display(char *display) {
  struct sigaction qsa;
  dpy = XOpenDisplay(display);
  if(!dpy) {
    fprintf(stderr, "error: can't open display\n");
    exit(1);
  }
  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);
  qsa.sa_handler = quit;
  sigemptyset(&qsa.sa_mask);
  qsa.sa_flags = 0;
  sigaction(SIGTERM, &qsa, NULL);
  sigaction(SIGINT, &qsa, NULL);
  sigaction(SIGHUP, &qsa, NULL);
}

void end(void) {
  while(cn)
    remove_client(0);
  free(clients);
  XCloseDisplay(dpy);
}

void quit(int sig) {
  end();
  exit(0);
}

int main(int argc, char *argv[]) {
  XEvent ev;
  XModifierKeymap *modmap;
  int i;
  open_display(0);
  XSetErrorHandler(&xerrorhandler);
  XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);
  xa_wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
  modmap = XGetModifierMapping(dpy);
  for(i = 0; i < 8; i++)
    if(modmap->modifiermap[modmap->max_keypermod * i] == XKeysymToKeycode(dpy, XK_Num_Lock))
      numlockmask = (1 << i);
  config_read();
  p_attr.override_redirect = True;
  p_attr.background_pixel = ifg.pixel;
  p_attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonReleaseMask | ExposureMask;
  taskbar = XCreateWindow(dpy, root, 0, 0, 1, 1, 0,
                          DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                          CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  p_attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | ExposureMask;
  p_attr.background_pixel = ibg.pixel;
  add_initial_clients();
  while(1) {
    XNextEvent(dpy, &ev);
    handle_event(ev);
  }
}

