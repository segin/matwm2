#include "matmenu.h"

int current = -1, p = 0, scroll = 0, c;
char buf[BUF_SIZE], *cc;

void draw() {
	int i, n = 0, skip = scroll;
	XClearWindow(dpy, input);
	if(current == -1)
		XDrawString(dpy, input, gc, 2, 2 + font->max_bounds.ascent, buf, p);
	c = 0;
	for(i = 0; i < ncomp; i++)
		if(strncmp(comp[i], buf, p) == 0) {
			if(skip) {
				skip--;
				continue;
			}
			if(n == ncw) {
				c++;
				continue;
			}
			if(n == 0 && current == -1)
				cc = comp[i];
			if(n == current) {
				XDrawString(dpy, input, gc, 2, 2 + font->max_bounds.ascent, comp[i], strlen(comp[i]));
				XSetWindowBackground(dpy, cw[n], bg.pixel);
				cc = comp[i];
			}
			XClearWindow(dpy, cw[n]);
			XDrawString(dpy, cw[n++], gc, 2, 2 + font->max_bounds.ascent, comp[i], strlen(comp[i]));
		}
	c += scroll + n;
	for(; n < ncw; n++)
		XClearWindow(dpy, cw[n]);
}

void complete(void) {
	if(current > -1 && strlen(cc) < BUF_SIZE) {
		XSetWindowBackground(dpy, cw[current], ibg.pixel);
		strncpy(buf, cc, strlen(cc));
		p = strlen(cc);
	 	current = -1;
		scroll = 0;
	}
}

void handle_event(XEvent ev) {
	int i, r;
	switch(ev.type) {
		case Expose:
			if(ev.xexpose.count == 0)
				draw();
			break;
		case KeyPress:
			r = XLookupString(&ev.xkey, buf + p, BUF_SIZE - p, NULL, NULL);
			if(keyaction(ev) == KA_CANCEL)
				exit(0);
			if(keyaction(ev) == KA_OK) {
				if(current == -1 && c)
					current++;
				complete();
				buf[p] = 0;
				printf(buf);
				exit(0);
			}
			if(keyaction(ev) == KA_DEL) {
				complete();
				if(p)
					p--;
			} else if(keyaction(ev) == KA_NEXT) {
				if(current < c - 1 && current < ncw - 1) {
					if(current > -1)
						XSetWindowBackground(dpy, cw[current], ibg.pixel);
					current++;
				} else if(scroll + current < c - 1)
					scroll++;
			} else if(keyaction(ev) == KA_PREV) {
				if(current > 0 || (!scroll && current == 0)) {
					XSetWindowBackground(dpy, cw[current], ibg.pixel);
					current--;
				} else if(scroll)
					scroll--;
			} else if(r && p + r < BUF_SIZE) {
				if(current > -1 && strlen(cc) < BUF_SIZE - r)
					strncpy(buf + strlen(cc), buf + p, r);
				complete();
				p += r;
			}
			draw();
			break;
	}
}

