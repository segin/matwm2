#include <xosd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <soundcard.h>
#include <X11/Xlib.h>

#ifndef __progname
	#define __progname "osd"
#endif

int main(int argc, char *argv[]) {
	xosd *osd;
	int fd, vol;
	Display *dpy = XOpenDisplay(NULL);
	Window root;

	if (!dpy) {
		fprintf(stderr, __progname ": error: cannot open display\n");
		exit(EXIT_FAILURE);
	}
	root = DefaultRootWindow(dpy);

	XGrabKey(dpy, 174, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, 176, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);

	fd = open("/dev/mixer0", O_RDWR);
	if(fd < 0) {
		perror(__progname);
		exit(1);
	}

	osd = xosd_create(1);
	xosd_set_font(osd, "-*-courier-*-r-*-*-34-*-*-*-*-*-*-*");
	xosd_set_timeout(osd, 1);
	xosd_set_shadow_offset(osd, 1);
	xosd_set_align(osd, XOSD_center);
	xosd_set_vertical_offset(osd, 650);

	while (1) {
		XEvent ev;
		XNextEvent(dpy, &ev);
		if (ev.type == KeyPress) {
			ioctl(fd, SOUND_MIXER_READ_VOLUME, &vol);
			vol = (vol * 100) / 25700;
			xosd_display(osd, 0, XOSD_percentage, vol);
		}
	}
}
