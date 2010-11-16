#include "matwm.h"

Display *dpy = NULL;
int screen, display_width, display_height, have_shape, shape_event, qsfd[2];
Window root;
Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints;
XSetWindowAttributes p_attr;
char *dn = NULL;
Colormap colormap;

int main(int argc, char *argv[]) {
	XEvent ev;
	int i;
	unsigned int ui, nwins;
	Window dw, *wins;
	XWindowAttributes attr;
	int dfd, di, sr;
	fd_set fds, fdsr;
	char act;
	/* parse command line arguments */
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-defaults") == 0) {
			for(ui = 0; ui < DEF_CFG_LINES; ui++)
				printf("%s\n", def_cfg[ui]);
			return 0;
		}
		if(strcmp(argv[i], "-version") == 0) {
			printf("%s version %s\n", NAME, VERSION);
			return 0;
		}
		if(strcmp(argv[i], "-display") == 0) {
			if(i + 1 >= argc) {
				fprintf(stderr, "error: argument -display needs an argument\n");
				return 1;
			}
 			dn = argv[i + 1];
			i++;
			continue;
		}
		fprintf(stderr, "error: argument %s not recognised\n", argv[i]);
		return 1;
	}
	/* setup signal handler etc */
	atexit(&quit);
	if(pipe(qsfd) != 0) /* we will use this pipe in the main loop, signal hander, etc */
		error();
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);
	signal(SIGHUP, &sighandler);
	signal(SIGUSR1, &sighandler);
	/* open connection with X and aquire some important info */
	dpy = XOpenDisplay(dn); /* if dn is NULL, XOpenDisplay() schould use the DISPLAY environment variable instead */
	if(!dpy) {
		fprintf(stderr, "error: can't open display \"%s\"\n", XDisplayName(dn));
		exit(1);
	}
#ifdef SYNC
	XSynchronize(dpy, True); /* synchronise all events, useful to make output from DEBUG_EVENTS more useful */
#endif
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	colormap = DefaultColormap(dpy, screen);
	XSetErrorHandler(&xerrorhandler); /* set up error handler - to be found in x11.c */
	XSelectInput(dpy, root, StructureNotifyMask | SubstructureRedirectMask | SubstructureNotifyMask); /* only one application can select these, this will error if another window manager is running */
	xa_wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
	xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
	xa_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
	xa_motif_wm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
	display_width = XDisplayWidth(dpy, screen);
	display_height = XDisplayHeight(dpy, screen);
	cfg_read(1); /* read configuration - see config.c */
	p_attr.override_redirect = True;
	p_attr.background_pixel = fg.pixel;
	p_attr.border_pixel = ifg.pixel;
	p_attr.event_mask = ExposureMask;
	wlist = XCreateWindow(dpy, root, 0, 0, 1, 1, 0, /* create the window list */
	                      DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                      CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	p_attr.event_mask = SubstructureRedirectMask |  SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | ExposureMask;
	p_attr.background_pixel = ibg.pixel;
#ifdef SHAPE
	have_shape = XShapeQueryExtension(dpy, &shape_event, &di);
#endif
	ewmh_initialize();
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime); /* set input focus to the root window */
	XQueryTree(dpy, root, &dw, &dw, &wins, &nwins); /* look for windows already present */
	for(ui = 0; ui < nwins; ui++) {
		if(XGetWindowAttributes(dpy, wins[ui], &attr))
			if(!attr.override_redirect && attr.map_state == IsViewable)
				client_add(wins[ui]);
	}
	if(wins != NULL)
		XFree(wins);
	ewmh_update_clist();
	dfd = ConnectionNumber(dpy); /* get the file descriptor Xlib uses to communicate with the server */
	FD_ZERO(&fds);
	FD_SET(qsfd[0], &fds);
	FD_SET(dfd, &fds);
	while(1)
		if(XPending(dpy)) { /* check if there are X events pending */
			XNextEvent(dpy, &ev);
			handle_event(ev);
		} else { /* no X events are pending do select() on X file descriptor and our pipe */
			fdsr = fds; /* reset fdsr */
			sr = select(((qsfd[0] < dfd) ? dfd : qsfd[0]) + 1, &fdsr, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) NULL);
			if(sr == -1 && errno != EINTR) /* we ignore EINTR - it will happen when we get a signal */
				error();
			if(sr > 0)
				if(FD_ISSET(qsfd[0], &fdsr)) /* we got data in our pipe */
					if(read(qsfd[0], &act, sizeof(act)) == sizeof(act)) {
						if(act == REINIT)
							cfg_reinitialize();
						else exit((act == ERROR) ? 1 : 0);
					}
			/* XPending will be called again next run of this loop so we just ignore its descriptor for now */
		}
}

void quit(void) {
	int i, d;
	d = dc;
	/* put windows back on the root window */
	while(d != -1) {
		if(d != desktop) /* first windows we don't see */
			for(i = cn - 1; i >= 0; i--)
				if(stacking[i]->desktop == d || (d == dc && stacking[i]->flags & ICONIC)) {
					client_deparent(stacking[i]);
					if(stacking[i]->flags & ICONIC) /* iconic windows are unmapped */
						XMapWindow(dpy, stacking[i]->window);
				}
		d--;
	}
	for(i = cn - 1; i >= 0; i--) /* on to of that go the currently visible ones */
		if(stacking[i]->desktop == desktop || stacking[i]->desktop == STICKY)
			client_deparent(stacking[i]);
	/* remove the client structures */
	while(cn)
		client_remove(stacking[cn - 1]);
	/* free allocated data */
	if(stacking)
		free((void *) stacking);
	if(clients)
		free((void *) clients);
	if(mod_ignore)
		free((void *) mod_ignore);
	if(buttons_left)
		free((void *) buttons_left);
	if(buttons_right)
		free((void *) buttons_right);
	keys_free();
	if(dpy) { /* if we have a connection with X, close it */
		XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
		XCloseDisplay(dpy);
	}
}

void qsfd_send(char s) { /* used to communicate with the main loop via our pipe */
	write(qsfd[1], &s, sizeof(s));
}

void error(void) { /* for functions that set errno on errors */
	perror("error");
	qsfd_send(ERROR);
}

void sighandler(int sig) {
	int act = QUIT;
	if(sig == SIGUSR1)
		act = REINIT;
	qsfd_send(act);
}

