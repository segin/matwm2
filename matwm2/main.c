#include "matwm.h"
#include <signal.h> /* for signal() */
#include <sys/select.h> /* for select() */
#include <errno.h> /* for checking select() errors */
/* for read() and write() */
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h> /* for waitpid(), read() and write() */
/* for waitpid */
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

Display *dpy = NULL;
int screen, depth, have_shape, shape_event, qsfd[2];
Window root;
Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints;
XSetWindowAttributes p_attr;
char *dn = NULL, *perror_str = NAME ": error";
Colormap colormap;
Visual *visual;

int main(int argc, char *argv[]) {
	XEvent ev;
	int i, dfd, sr;
	unsigned int ui, nwins;
	Window w, dw, *wins;
	XWindowAttributes attr;
	#ifdef USE_SHAPE
	int di;
	#endif
	fd_set fds, fdsr;
	char act;
	client *c;
	/* parse command line arguments */
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-defaults") == 0) {
			for(ui = 0; ui < DEF_CFG_LINES; ui++)
				printf("%s\n", def_cfg[ui]);
			return 0;
		}
		if(strcmp(argv[i], "-version") == 0) {
			printf(NAME " version " VERSION "\n"
				"options set at compile time:\n"
				"\tXft support: "
				#ifdef USE_XFT
				"enabled"
				#else
				"disabled"
				#endif
				"\n\tshaped windows support: "
				#ifdef USE_SHAPE
				"enabled"
				#else
				"disabled"
				#endif
				"\n\tuse vfork system call: "
				#ifdef HAVE_VFORK
				"enabled"
				#else
				"disabled"
				#endif
				"\n\tdebugging output: "
				#ifdef DEBUG
				"enabled"
				#else
				"disabled"
				#endif
				"\n\tprint all received events: "
				#ifdef DEBUG_EVENTS
				"enabled"
				#else
				"disabled"
				#endif
				"\n\tsynchronise events: "
				#ifdef SYNC
				"enabled"
				#else
				"disabled"
				#endif
				"\n"
			);
			return 0;
		}
		if(strcmp(argv[i], "-display") == 0) {
			if(i + 1 >= argc) {
				fprintf(stderr, NAME ": error: argument -display needs an argument\n");
				return 1;
			}
 			dn = argv[i + 1];
			i++;
			continue;
		}
		fprintf(stderr, NAME ": error: argument %s not recognised\n", argv[i]);
		return 1;
	}
	/* setup signal handler etc */
	if(pipe(qsfd) != 0) /* we will use this pipe in the main loop, signal hander, etc */
		error();
	atexit(&quit);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);
	signal(SIGHUP, &sighandler);
	signal(SIGUSR1, &sighandler);
	signal(SIGCHLD, &sighandler);
	/* open connection with X and aquire some important info */
	dpy = XOpenDisplay(dn); /* if dn is NULL, XOpenDisplay() schould use the DISPLAY environment variable instead */
	if(!dpy) {
		fprintf(stderr, NAME ": error: can't open display \"%s\"\n", XDisplayName(dn));
		exit(1);
	}
	#ifdef SYNC
	XSynchronize(dpy, True); /* synchronise all events, useful to make output from DEBUG_EVENTS more useful */
	#endif
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	colormap = DefaultColormap(dpy, screen);
	depth = DefaultDepth(dpy, screen);
	visual = DefaultVisual(dpy, screen);
	XSetErrorHandler(&xerrorhandler); /* set up error handler - to be found in x11.c */
	xa_wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
	xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
	xa_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
	xa_motif_wm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
	ewmh_initialize();
	screens_get(); /* we need atoms from above XInternAtom() and ewmh_initialize() calls for this */
	cfg_read(1); /* read configuration - see config.c */
	/* select events on the root window */
	if(!select_root_events()) /* config has to be read before this */
		exit(1);
	/* create window list window */
	p_attr.override_redirect = True;
	p_attr.background_pixel = fg.pixel;
	p_attr.border_pixel = ibfg.pixel;
	p_attr.event_mask = ExposureMask;
	wlist = XCreateWindow(dpy, root, 0, 0, 1, 1, 0, /* create the window list */
	                      DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                      CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	/* set attributes for further use */
	p_attr.background_pixel = ibg.pixel;
	/* update EWMH hints */
	ewmh_update(); /* for this we need wlist to be there and configuration to be read */
	#ifdef USE_SHAPE
	/* get info about shape extension */
	have_shape = XShapeQueryExtension(dpy, &shape_event, &di);
	#endif
	/* look for windows that are already present */
	XGetInputFocus(dpy, &w, &i); /* next step might change focus */
	XQueryTree(dpy, root, &dw, &dw, &wins, &nwins);
	for(ui = 0; ui < nwins; ui++)
		if(XGetWindowAttributes(dpy, wins[ui], &attr)) {
			if(!attr.override_redirect && attr.map_state == IsViewable)
				client_add(wins[ui], true);
			else if(wins[ui] != wlist) XRaiseWindow(dpy, wins[ui]);
		}
	if(wins != NULL)
		XFree(wins);
	/* look what window is to be focussed (if one) */
	if(w == PointerRoot || w == root || w == None)
		XQueryPointer(dpy, root, &dw, &w, &di, &di, &di, &di, &ui);
	if(w != None && w != PointerRoot && w != root) {
		c = owner(w);
		if(c) client_focus(c, true);
		else XSetInputFocus(dpy, w, RevertToPointerRoot, CurrentTime);
	}	else {
		client_focus_first();
		if(!current) /* if input focus is set to None, input doesn't work at all */
			XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	}
	/* remove some events we might have generated */
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, FocusChangeMask | EnterWindowMask, &ev));
	/* update EWMH client stuff */
	ewmh_update_clist();
	/* initialize file descriptor set for select() */
	dfd = ConnectionNumber(dpy); /* gets the file descriptor Xlib uses to communicate with the server */
	FD_ZERO(&fds);
	FD_SET(qsfd[0], &fds);
	FD_SET(dfd, &fds);
	/* our main loop */
	while(1)
		if(XPending(dpy)) { /* check if there are X events pending */
			XNextEvent(dpy, &ev);
			handle_event(&ev);
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
						else exit(0);
					}
			/* XPending will be called again next run of this loop so we just ignore its descriptor for now */
		}
}

void quit(void) {
	int d, i;
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
	for(i = cn - 1; i >= 0; i--) /* on top of that go the currently visible ones */
		if(stacking[i]->desktop == desktop || stacking[i]->desktop == STICKY)
			client_deparent(stacking[i]);
	if(dpy) { /* if we have a connection with X, close it */
		XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime); /* this prevents focus being stuck with no WM running */
		XCloseDisplay(dpy);
	}
}

void qsfd_send(char s) { /* used to communicate with the main loop via our pipe */
	write(qsfd[1], &s, sizeof(s));
}

void sighandler(int sig) {
	char act = QUIT;
	int status;
	if(sig == SIGCHLD) {
		waitpid(-1, &status, WNOHANG);
		return;
	}
	if(sig == SIGUSR1)
		act = REINIT;
	qsfd_send(act);
}
