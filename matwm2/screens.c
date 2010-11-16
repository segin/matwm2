#include "matwm.h"
#ifdef USE_XINERAMA
#include<X11/extensions/Xinerama.h>
#endif

screen_dimensions *screens = NULL;
int nscreens = 0, cs = 0;

bool screens_handle_event(XEvent *ev) {
	int i;
	if(ev->type == ConfigureNotify && ev->xconfigure.window == root) {
		screens_get();
		if(evh == wlist_handle_event)
			wlist_update();
		ewmh_update_geometry();
		for(i = 0; i < cn; i++)
			client_update_size(clients[i]);
		return true;
	}
	return false;
}

void screens_get(void) {
	#ifdef USE_XINERAMA
	int i, event, error;
	XineramaScreenInfo *screeninfo;
	if(XineramaQueryExtension(dpy, &event, &error)) {
		screeninfo = XineramaQueryScreens(dpy, &nscreens);
		if(nscreens) {
			screens = _realloc((void *) screens, nscreens * sizeof(screen_dimensions));
			for(i = 0; i < nscreens; i++) {
				screens[i].x = screeninfo[i].x_org;
				screens[i].y = screeninfo[i].y_org;
				screens[i].width = screeninfo[i].width;
				screens[i].height = screeninfo[i].height;
			}
			XFree((void *) screeninfo);
			goto ok;
		}
	}
	#endif
	nscreens = 1;
	screens = _realloc(screens, sizeof(screen_dimensions));
	screens[0].x = 0;
	screens[0].y = 0;
	screens[0].width = XDisplayWidth(dpy, screen);
	screens[0].height = XDisplayHeight(dpy, screen);
	ok:
	screens_update_current();
	ewmh_update_geometry();
	ewmh_update_strut();
	if(wlist_screen > nscreens)
		wlist_screen = nscreens - 1;
}

void screens_update_current(void) {
	Window dw;
	int x, y, i;
	unsigned int dui;
	if(XQueryPointer(dpy, root, &dw, &dw, &x, &y, &i, &i, &dui))
		for(i = 0; i < nscreens; i++)
			if(x > screens[i].x && y > screens[i].y && x < screens[i].x + screens[i].width && y < screens[i].y + screens[i].height) {
				cs = i;
				return;
			}
}

int intersect(int base_start, int base_len, int start, int len) { /* calculates how much of the line defined by start and len overlaps with the line defined by base_start and base_len */
	int ret = 0;
	/* make a coördinate out of length values */
	base_len += base_start;
	len += start;
	/* check how much of our line overlaps the base line */
	if(start < base_len && len > base_start) {
		ret += (len < base_len) ? len : base_len;
		ret -= (start > base_start) ? start : base_start;
	}
	return ret;
}

void client_update_screen(client *c) {
	int i, area = 0, carea;
	int scr = 0;
	for(i = 0; i < nscreens; i++) {
		carea = intersect(client_x(c), client_width_total(c), screens[i].x, screens[i].width) * intersect(client_y(c), client_height_total(c), screens[i].y, screens[i].height);
		if(carea > area) {
			scr = i;
			area = carea;
		}
	}
	c->screen = scr;
}

int screens_leftmost(void) { /* returns the leftmost coordinate of all screens */
	int i, ret = screens[0].x;
	for(i = 1; i < nscreens; i++) {
		if(ret > screens[i].x)
			ret = screens[i].x;
	}
	return ret;
}

int screens_rightmost(void) { /* returns the rightmost coordinate of all screens */
	int i, c, ret = screens[0].x + screens[0].width;
	for(i = 1; i < nscreens; i++) {
		c = screens[i].x + screens[i].width;
		if(c > ret)
			ret = c;
	}
	return ret;
}

int screens_topmost(void) { /* returns the topmost coordinate of all screens */
	int i, ret = screens[0].y;
	for(i = 1; i < nscreens; i++) {
		if(ret > screens[i].y)
			ret = screens[i].y;
	}
	return ret;
}

int screens_bottom(void) { /* returns the lowest bottom coordinate of all screens */
	int i, c, ret = screens[0].y + screens[0].height;
	for(i = 1; i < nscreens; i++) {
		c = screens[i].y + screens[i].height;
		if(c > ret)
			ret = c;
	}
	return ret;
}

bool screens_correct_center(int *x, int *y, int *width, int *height) { /* for windows that try to place windows centered without knowing there are multiple screens */
	bool ret = false;
	int min, max, ref;
	screens_update_current();
	min = *x + (*width / 2) - 2;
	max = min + 4;
	ref = (screens_rightmost() - screens_leftmost()) / 2;
	if(*width < screens[cs].width && ref > min && ref < max) {
		*x = screens[cs].x + ((screens[cs].width / 2) - (*width / 2));
		ret = true;
	}
	min = *y + (*height / 2) - 1;
	max = min + 2;
	ref = (screens_bottom() - screens_topmost()) / 2;
	if(*height < screens[cs].height && ref > min && ref < max) {
		*y = screens[cs].y + ((screens[cs].height / 2) - (*height / 2));
		ret = true;
	}
	return ret;
}
