/* Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by Mattis Michel.
 * 4. Neither the name 'Mattis Michel' nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* needs OSS and crappy (read: GPLd) xosd library
 * compile with:
 * cc osd.c -o osd `xosd-config --libs --cflags xosd` `pkg-config --libs --cflags x11` -lossaudio
 */
#include <xosd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <soundcard.h>
#include <X11/Xlib.h>

#define Y_OFFSET 700

#ifndef __progname
	#define __progname "volosd"
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

	/* modify keycodes for other lappies */
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
	xosd_set_vertical_offset(osd, Y_OFFSET);

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
