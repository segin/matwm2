#include "matwm.h"

Display *dpy;
int screen, display_height;
Window root;
unsigned int numlockmask = 0;
Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state;
XSetWindowAttributes p_attr;
XModifierKeymap *modmap;

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
  while(cn) {
    if(clients[0].iconic)
      XMapWindow(dpy, clients[0].window);
    remove_client(0, 0);
  }
  free(clients);
  XCloseDisplay(dpy);
}

void quit(int sig) {
  end();
  exit(0);
}

int main(int argc, char *argv[]) {
  XEvent ev;
  unsigned int i, nwins;
  Window dw1, dw2, *wins;
  XWindowAttributes attr;
  open_display(0);
  XSetErrorHandler(&xerrorhandler);
  XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);
  xa_wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
  xa_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
  modmap = XGetModifierMapping(dpy);
  for(i = 0; i < 8; i++)
    if(modmap->modifiermap[modmap->max_keypermod * i] == XKeysymToKeycode(dpy, XK_Num_Lock))
      numlockmask = (1 << i);
  config_read();
  p_attr.override_redirect = True;
  p_attr.background_pixel = ibg.pixel;
  p_attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | ExposureMask;
  display_height = XDisplayHeight(dpy, screen);
  XQueryTree(dpy, root, &dw1, &dw2, &wins, &nwins);
  for(i = 0; i < nwins; i++) {
    XGetWindowAttributes(dpy, wins[i], &attr);
    if(!attr.override_redirect && attr.map_state == IsViewable)
      add_client(wins[i], 0);
  }
  if(wins != NULL)
    XFree(wins);
  while(1) {
    XNextEvent(dpy, &ev);
    handle_event(ev);
  }
}

