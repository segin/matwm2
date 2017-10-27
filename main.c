#include "matwm.h"
#include <signal.h> /* for signal() */
/* for waitpid */
#include <sys/types.h>
#ifndef _WIN32
# include <sys/wait.h>
# include <sys/resource.h>
#endif
#include <sys/time.h>

Display *dpy = NULL;
int screen, depth, have_shape, shape_event;
Window root;
Atom xa_wm_protocols, xa_wm_delete, xa_wm_take_focus, xa_wm_state, xa_wm_change_state, xa_motif_wm_hints, xa_internal_message, xa_quit, xa_reinit, xa_utf8_string;
XSetWindowAttributes p_attr;
char *dn = NULL, *perror_str = NAME ": error";
Colormap colormap;
Visual *visual;

int main(int argc, char *argv[]) {
	XEvent ev;
	int i, di;
	unsigned int ui, nwins;
	Window w, dw, *wins;
	XWindowAttributes attr;
	client *c;
	/* parse command line arguments */
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-defaults") == 0) {
			for(ui = 0; ui < DEF_CFG_LINES; ui++)
				printf("%s\n", def_cfg[ui]);
			return EXIT_SUCCESS;
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
				"\n\tdebugging output about events: "
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
			return EXIT_SUCCESS;
		}
		if(strcmp(argv[i], "-display") == 0) {
			if(i + 1 >= argc) {
				fprintf(stderr, NAME ": error: argument -display needs an argument\n");
				return EXIT_FAILURE;
			}
 			dn = argv[i + 1];
			i++;
			continue;
		}
		fprintf(stderr, NAME ": error: argument %s not recognised\n", argv[i]);
		return EXIT_FAILURE;
	}
	/* open connection with X and aquire some important info */
	dpy = XOpenDisplay(dn); /* if dn is NULL, XOpenDisplay() schould use the DISPLAY environment variable instead */
	if(!dpy) {
		fprintf(stderr, NAME ": error: can't open display \"%s\"\n", XDisplayName(dn));
		exit(EXIT_FAILURE);
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
	xa_wm_take_focus = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
	xa_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
	xa_motif_wm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
	xa_internal_message = XInternAtom(dpy, XA_INTERNAL_MESSAGE, False);
	xa_quit = XInternAtom(dpy, XA_QUIT, False);
	xa_reinit = XInternAtom(dpy, XA_REINIT, False);
	xa_motif_wm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
	xa_utf8_string = XInternAtom(dpy, "UTF8_STRING", False);
	/* load configuration etc */
	ewmh_initialize();
	screens_get(); /* we need atoms from above XInternAtom() and ewmh_initialize() calls for this */
	cfg_read(1); /* read configuration - see config.c */
	/* select events on the root window */
	if(!select_root_events()) /* config has to be read before this */
		exit(EXIT_FAILURE);
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
	/* setup signal handler etc */
	atexit(&quit);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);
#ifndef _WIN32	
	signal(SIGHUP, &sighandler);
	signal(SIGUSR1, &sighandler);
	signal(SIGCHLD, &sighandler);
#endif 	
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
	/* our main loop */
	while(1) {
		XNextEvent(dpy, &ev);
		handle_event(&ev);
	}
}

void quit(void) {
	int d, i;
	d = dc;
	#ifdef DEBUG
	fprintf(stderr, NAME ": quit(): quitting...\n");
	#endif
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

void sighandler(int sig) {
	XEvent ev;
	Window sh_root;
	Display *sh_display;
	int status;
#ifndef _WIN32
	if(sig == SIGCHLD) {
		waitpid(-1, &status, WNOHANG);
		return;
	}
#endif
	sh_display = XOpenDisplay(dn); /* we can't use our other display, cause we are very probally in the middle of a XNextEvent() call */
	if(!sh_display) {
		fprintf(stderr, NAME ": sighandler(): error: can't open display \"%s\"\n", XDisplayName(dn));
		fprintf(stderr, "\tsomething went horribly wrong, if you are trying to quit: use SIGKILL instead\n");
		return;
	}
	sh_root = RootWindow(sh_display, DefaultScreen(sh_display));
	ev.type = ClientMessage;
	ev.xclient.window = wlist;
	ev.xclient.message_type = XInternAtom(sh_display, XA_INTERNAL_MESSAGE, False);
	ev.xclient.format = 32;
#ifndef _WIN32
	ev.xclient.data.l[0] = XInternAtom(sh_display, (sig == SIGUSR1) ? XA_REINIT : XA_QUIT, False);
#endif
	XSendEvent(sh_display, wlist, False, NoEventMask, &ev);
	XFlush(sh_display);
	XSync(sh_display, False);
	XCloseDisplay(sh_display);
	#ifdef DEBUG
	printf(NAME ": sighandler(): quit message sent\n");
	#endif
}
