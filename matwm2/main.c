#include "matwm.h"
#include <signal.h>
#include <stdio.h>

Display *dpy;
int screen;
Window root;
Atom wm_protocols, wm_delete, wm_state;

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
    XReparentWindow(dpy, clients->window, root, clients->x, clients->y);
    remove_client(0);
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
  open_display(0);
  XSetErrorHandler(&xerrorhandler);
  config_read();
  add_initial_clients();
  wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  wm_state = XInternAtom(dpy, "WM_STATE", False);
  XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);
  while(1) {
    XNextEvent(dpy, &ev);
    handle_event(ev);
  }
}

