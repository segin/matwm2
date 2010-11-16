#include "matwm.h"

Display *dpy;
int screen, display_width, display_height, have_shape, shape_event, qsfd[2];
Window root;
Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints;
XSetWindowAttributes p_attr;

void error(void) {
  perror("error");
  exit(1);
}

void end(void) {
  while(cn) {
    if(clients[0].iconic)
      XMapWindow(dpy, clients[0].window);
    remove_client(0, 0);
  }
  if(clients)
    free(clients);
  XCloseDisplay(dpy);
}

void quit(int sig) {
  write(qsfd[1], &sig, sizeof(int));
}

int main(int argc, char *argv[]) {
  XEvent ev;
  unsigned int nwins;
  Window dw, *wins;
  XWindowAttributes attr;
  struct sigaction qsa;
  int dfd, i, di, sr;
  fd_set fds, fdsr;
  dpy = XOpenDisplay(0);
  if(!dpy) {
    fprintf(stderr, "error: can't open display %s\n", XDisplayName(0));
    exit(1);
  }
  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);
  atexit(&end);
  if(pipe(qsfd) != 0)
    error();
  qsa.sa_handler = quit;
  sigemptyset(&qsa.sa_mask);
  qsa.sa_flags = 0;
  sigaction(SIGTERM, &qsa, NULL);
  sigaction(SIGINT, &qsa, NULL);
  sigaction(SIGHUP, &qsa, NULL);
  XSetErrorHandler(&xerrorhandler);
  XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);
  xa_wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
  xa_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
  xa_motif_wm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
  display_width = XDisplayWidth(dpy, screen);
  display_height = XDisplayHeight(dpy, screen);
  config_read();
  p_attr.override_redirect = True;
  p_attr.background_pixel = ibg.pixel;
  p_attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | ExposureMask;
  have_shape = XShapeQueryExtension(dpy, &shape_event, &di);
  XQueryTree(dpy, root, &dw, &dw, &wins, &nwins);
  for(i = 0; i < nwins; i++) {
    XGetWindowAttributes(dpy, wins[i], &attr);
    if(!attr.override_redirect && attr.map_state == IsViewable)
      add_client(wins[i]);
  }
  if(wins != NULL)
    XFree(wins);
  dfd = ConnectionNumber(dpy);
  FD_ZERO(&fds);
  FD_SET(qsfd[0], &fds);
  FD_SET(dfd, &fds);
  while(1) {
    if(!XPending(dpy)) {
      fdsr = fds;
      sr = select((qsfd[0] < dfd ? dfd : qsfd[0]) + 1, &fdsr, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) NULL);
      if(sr == -1 && errno != EINTR)
        error();
      if(sr == 0)
        continue;
      if(FD_ISSET(qsfd[0], &fdsr))
        exit(1);
    }
    XNextEvent(dpy, &ev);
    handle_event(ev);
  }
}

