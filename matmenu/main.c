#include "matmenu.h"

Display *dpy;
int screen, display_width, display_height, qsfd[2], ncomp = 0;
Window root, menu, input, *cw;
Atom xa_wm_protocols, xa_wm_delete, xa_wm_state, xa_wm_change_state;
XSetWindowAttributes p_attr;
char *dn = NULL, **comp = NULL;

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
			break;
		}
		if(strcmp(argv[i], "-width") == 0 && i + 1 < argc) {
			menu_width = strtol(argv[i + 1], NULL, 0);
			break;
		}
		if(strcmp(argv[i], "-height") == 0 && i + 1 < argc) {
			ncw = strtol(argv[i + 1], NULL, 0);
			break;
		}
		fprintf(stderr, "error: argument %s not recognised\n", argv[i]);
		return 1;
	}
	readcomp(0);
	dpy = XOpenDisplay(dn);
	if(!dpy) {
		fprintf(stderr, "error: can't open display \"%s\"\n", XDisplayName(dn));
		exit(1);
	}
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	atexit(&end);
	if(pipe(qsfd) != 0)
		error();
	qsa.sa_handler = qsh;
	sigemptyset(&qsa.sa_mask);
	qsa.sa_flags = 0;
	sigaction(SIGTERM, &qsa, NULL);
	sigaction(SIGINT, &qsa, NULL);
	sigaction(SIGHUP, &qsa, NULL);
	XSetErrorHandler(&xerrorhandler);
	xa_wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
	xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
	xa_wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);
	display_width = XDisplayWidth(dpy, screen);
	display_height = XDisplayHeight(dpy, screen);
	cfg_read();
	p_attr.override_redirect = True;
	p_attr.background_pixel = fg.pixel;
	p_attr.event_mask = ExposureMask | KeyPressMask;
	menu = XCreateWindow(dpy, root, (display_width / 2) - (menu_width / 2), (display_height / 2) - (menu_height / 2), menu_width, menu_height, 0,
												DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
												CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	p_attr.background_pixel = bg.pixel;
	input = XCreateWindow(dpy, menu, 1, 1, 300 - 2, item_height, 0,
												DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
												CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	p_attr.background_pixel = ibg.pixel;
	XMapWindow(dpy, input);
	cw = (Window *) malloc(sizeof(Window) * ncw);
	for(i = 0; i < ncw; i++) {
		cw[i] = XCreateWindow(dpy, menu, 1, ((item_height + 1) * (i + 1)) + 1, menu_width - 2, item_height, 0,
													DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
													CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
		XMapWindow(dpy, cw[i]);
	}
	XMapRaised(dpy, menu);
	while(XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime) != GrabSuccess)
		usleep(1000);
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

void end(void) {
	XUngrabKeyboard(dpy, CurrentTime);
	XCloseDisplay(dpy);
}

void error(void) {
	perror("error");
	exit(1);
}

void qsh(int sig) {
	write(qsfd[1], &sig, sizeof(int));
}

