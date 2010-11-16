#include "matwm.h"

Display *dpy;
int screen, display_width, display_height, have_shape, shape_event, qsfd[2];
Window root;
Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints;
XSetWindowAttributes p_attr;
char *dn = NULL;

int main(int argc, char *argv[]) {
  XEvent ev;
  unsigned int i, nwins;
  Window dw, *wins;
  XWindowAttributes attr;
  struct sigaction qsa;
  int dfd, di, sr;
  fd_set fds, fdsr;
  for(i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-defaults") == 0) {
      printf(DEF_CFG);
      return 0;
    }
    if(strcmp(argv[i], "-display") == 0 && i + 1 < argc) {
      dn = argv[i + 1];
      i++;
      continue;
    }
    fprintf(stderr, "error: argument %s not recognised\n", argv[i]);
    return 1;
  }
  dpy = XOpenDisplay(dn);
  if(!dpy) {
    fprintf(stderr, "error: can't open display \"%s\"\n", XDisplayName(dn));
    exit(1);
  }
  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);
  atexit(&quit);
  if(pipe(qsfd) != 0)
    error();
  qsa.sa_handler = qsh;
  sigemptyset(&qsa.sa_mask);
  qsa.sa_flags = 0;
  sigaction(SIGTERM, &qsa, NULL);
  sigaction(SIGINT, &qsa, NULL);
  sigaction(SIGHUP, &qsa, NULL);
#ifdef SYNC
  XSynchronize(dpy, True);
#endif
  XSetErrorHandler(&xerrorhandler);
  XSelectInput(dpy, root, StructureNotifyMask | SubstructureRedirectMask | SubstructureNotifyMask);
  xa_wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
  xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
  xa_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
  xa_motif_wm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
  display_width = XDisplayWidth(dpy, screen);
  display_height = XDisplayHeight(dpy, screen);
  cfg_read();
  p_attr.override_redirect = True;
  p_attr.background_pixel = fg.pixel;
  p_attr.border_pixel = ifg.pixel;
  p_attr.event_mask = ExposureMask | KeyReleaseMask;
  wlist = XCreateWindow(dpy, root, 0, 0, 1, 1, 0,
                        DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                        CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  p_attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | ExposureMask;
  p_attr.background_pixel = ibg.pixel;
  have_shape = XShapeQueryExtension(dpy, &shape_event, &di);
  ewmh_initialize();
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XQueryTree(dpy, root, &dw, &dw, &wins, &nwins);
  for(i = 0; i < nwins; i++) {
    XGetWindowAttributes(dpy, wins[i], &attr);
    if(!attr.override_redirect && attr.map_state == IsViewable)
      client_add(wins[i]);
  }
  if(wins != NULL)
    XFree(wins);
  ewmh_update_clist();
  dfd = ConnectionNumber(dpy);
  FD_ZERO(&fds);
  FD_SET(qsfd[0], &fds);
  FD_SET(dfd, &fds);
  while(1)
    if(XPending(dpy)) {
      XNextEvent(dpy, &ev);
      handle_event(ev);
    } else {
      fdsr = fds;
      sr = select((qsfd[0] < dfd ? dfd : qsfd[0]) + 1, &fdsr, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) NULL);
      if(sr == -1 && errno != EINTR)
        error();
      if(sr && FD_ISSET(qsfd[0], &fdsr))
        exit(0);
    }
}

void quit(void) {
  int i, d;
  d = ICONS;
  while(d != 0) {
    if(d != desktop)
      for(i = cn - 1; i >= 0; i--)
        if(stacking[i]->desktop == d) {
          client_deparent(stacking[i]);
          XMapWindow(dpy, stacking[i]->window);
        }
    if(d == ICONS)
      d = dc;
    else d--;
  }
  for(i = cn - 1; i >= 0; i--)
    if(stacking[i]->desktop == desktop || stacking[i]->desktop == STICKY)
      client_deparent(stacking[i]);
  while(cn)
    client_remove(stacking[cn - 1]);
  if(stacking)
    free((void *) stacking);
  if(clients)
    free((void *) clients);
  keys_free();
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XCloseDisplay(dpy);
}

void error(void) {
  perror("error");
  exit(1);
}

void qsh(int sig) {
  write(qsfd[1], &sig, sizeof(int));
}

