#include "matwm.h"

Window button_current = None;
int button_down = 0;

void buttons_create(client *c) {
	c->button_parent = XCreateWindow(dpy, c->parent, (c->width + border_width - 1) - button_parent_width, border_width - 1, button_parent_width, button_size, 0,
	                                 DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                                 CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	c->button_iconify = XCreateWindow(dpy, c->button_parent, 0, 0, button_size, button_size, 0,
	                                  DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                                  CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	c->button_expand = XCreateWindow(dpy, c->button_parent, button_size + 2, 0, button_size, button_size, 0,
	                                 DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                                 CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	c->button_maximize = XCreateWindow(dpy, c->button_parent, ((button_size + 2) * 2), 0, button_size, button_size, 0,
	                                   DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                                   CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	c->button_close = XCreateWindow(dpy, c->button_parent, ((button_size + 2) * 3), 0, button_size, button_size, 0,
	                                DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                                CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	XMapWindow(dpy, c->button_parent);
	XMapWindow(dpy, c->button_iconify);
	XMapWindow(dpy, c->button_expand);
	XMapWindow(dpy, c->button_maximize);
	XMapWindow(dpy, c->button_close);
}

void buttons_draw(client *c) {
	button_draw(c, c->button_iconify);
	button_draw(c, c->button_expand);
	button_draw(c, c->button_maximize);
	button_draw(c, c->button_close);
}

void button_draw(client *c, Window b) {
	XClearWindow(dpy, b);
	if(button_current != None)
		XDrawRectangle(dpy, button_current, gc, 0, 0, button_size - 1, button_size - 1);
	if(b == c->button_iconify)
		XDrawRectangle(dpy, c->button_iconify, (c == current) ? gc : igc, (button_size / 2) - 1, (button_size / 2) - 1, 2, 2);
	if(b == c->button_expand) {
		XDrawLine(dpy, c->button_expand, (c == current) ? gc : igc, button_size / 2, 3, button_size / 2, button_size - 3);
		XDrawLine(dpy, c->button_expand, (c == current) ? gc : igc, 3, button_size / 2, button_size - 3, button_size / 2);
	}
	if(b == c->button_maximize)
		XDrawRectangle(dpy, c->button_maximize, (c == current) ? gc : igc, 2, 2, button_size - 5, button_size - 5);
	if(b == c->button_close) {
		XDrawLine(dpy, c->button_close, (c == current) ? gc : igc, 2, 2, button_size - 2, button_size - 2);
		XDrawLine(dpy, c->button_close, (c == current) ? gc : igc, 2, button_size - 3, button_size - 2, 1);
	}
}

void buttons_update(client *c) {
	if(c->flags & HAS_BUTTONS && c->width <= button_parent_width + 2) {
		c->flags ^= HAS_BUTTONS;
		XUnmapWindow(dpy, c->button_parent);
	} else if(!(c->flags & HAS_BUTTONS) && c->width > button_parent_width + 2) {
		c->flags |= HAS_BUTTONS;
		XMapWindow(dpy, c->button_parent);
	}
}

int button_handle_event(XEvent ev) {
	int i;
	client *c = NULL;
	for(i = 0; i < cn; i++)
		if(clients[i]->button_iconify == ev.xany.window || clients[i]->button_expand == ev.xany.window || clients[i]->button_maximize == ev.xany.window || clients[i]->button_close == ev.xany.window)
			c = clients[i];
	if(!c || !has_child(c->parent, c->window))
		return 0;
	switch(ev.type) {
		case Expose:
			button_draw(c, ev.xexpose.window);
			return 1;
		case EnterNotify:
			if(button_down) {
				button_down = 2;
				return 1;
			}
			button_current = ev.xcrossing.window;
			button_draw(c, ev.xcrossing.window);
			return 1;
		case LeaveNotify:
			if(button_down == 2)
				button_down = 1;
			button_current = None;
			button_draw(c, ev.xcrossing.window);
			return 1;
		case ButtonPress:
			if(ev.xbutton.button == Button1 || ev.xbutton.button == Button3)
				button_down = 1;
			return 1;
		case ButtonRelease:
			if(ev.xbutton.button == Button1 || ev.xbutton.button == Button3) {
				if(button_current == ev.xbutton.window) {
					if(ev.xbutton.window == c->button_iconify)
						client_iconify(c);
					if(ev.xbutton.window == c->button_expand)
						client_expand(c, EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B, 0);
					if(ev.xbutton.window == c->button_maximize)
						client_toggle_state(c, MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B);
					if(ev.xbutton.window == c->button_close)
						delete_window(c);
				}
				if(button_down == 2) {
					button_current = ev.xbutton.window;
					button_draw(c, ev.xbutton.window);
				}
				button_down = 0;
			}
			return 1;
	}
	return 0;
}

