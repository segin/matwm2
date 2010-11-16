#include "matwm.h"
#include <signal.h>
#include <stdio.h>

Display *dpy;
int screen;
Window root;
Atom wm_protocols, wm_delete;

void open_display() {
  struct sigaction qsa;

  dpy = XOpenDisplay(0);
  if(!dpy) {
    fprintf(stderr, "error: can't open display %s\n", XDisplayName(0));
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

void quit(int sig) {
  while(cn) {
    XReparentWindow(dpy, clients->window, root, clients->x, clients->y);
    remove_client(0);
  }
  free(clients);
  XCloseDisplay(dpy);
  exit(0);
}

int main(int argc, char *argv[]) {
  XEvent ev;
  Window dw1, dw2, *wins;
  int nwins;
#ifdef FREEBSD_MALLOC_DEBUG
  _malloc_options = "X";
#endif
  open_display();
  XSetErrorHandler(&xerrorhandler);
  /* Detects other WM, such as Xfwm4 */
  XQueryTree(dpy, root, &dw1, &dw2, &wins, &nwins);
  config_read();
  init_input();
  add_initial_clients();
  wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | PropertyChangeMask | ButtonPressMask);
  while(1) {
    XNextEvent(dpy, &ev);
    handle_event(ev);
  }
}

