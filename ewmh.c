#include "matwm.h"

Atom ewmh_atoms[EWMH_ATOM_COUNT];

void ewmh_initialize(void) {
	Atom rt;
	int rf;
	unsigned long nir, bar;
	unsigned char *p;
	long d;
	ewmh_atoms[NET_SUPPORTED] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	ewmh_atoms[NET_SUPPORTING_WM_CHECK] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	ewmh_atoms[NET_WM_NAME] = XInternAtom(dpy, "_NET_WM_NAME", False);
	ewmh_atoms[NET_NUMBER_OF_DESKTOPS] = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	ewmh_atoms[NET_DESKTOP_GEOMETRY] = XInternAtom(dpy, "_NET_DESKTOP_GEOMETRY", False);
	ewmh_atoms[NET_DESKTOP_VIEWPORT] = XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", False);
	ewmh_atoms[NET_WORKAREA] = XInternAtom(dpy, "_NET_WORKAREA", False);
	ewmh_atoms[NET_WM_STRUT] = XInternAtom(dpy, "_NET_WM_STRUT", False);
	ewmh_atoms[NET_WM_STRUT_PARTIAL] = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
	ewmh_atoms[NET_CURRENT_DESKTOP] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
	ewmh_atoms[NET_DESKTOP_NAMES] = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	ewmh_atoms[NET_WM_DESKTOP] = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
	ewmh_atoms[NET_CLIENT_LIST] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	ewmh_atoms[NET_CLIENT_LIST_STACKING] = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
	ewmh_atoms[NET_ACTIVE_WINDOW] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	ewmh_atoms[NET_WM_STATE] = XInternAtom(dpy, "_NET_WM_STATE", False);
	ewmh_atoms[NET_WM_STATE_FULLSCREEN] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	ewmh_atoms[NET_WM_STATE_MAXIMIZED_HORZ] = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	ewmh_atoms[NET_WM_STATE_MAXIMIZED_VERT] = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	ewmh_atoms[NET_WM_STATE_ABOVE] = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
	ewmh_atoms[NET_WM_STATE_BELOW] = XInternAtom(dpy, "_NET_WM_STATE_BELOW", False);
	ewmh_atoms[NET_CLOSE_WINDOW] = XInternAtom(dpy, "_NET_CLOSE_WINDOW", False);
	ewmh_atoms[NET_WM_WINDOW_TYPE] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	ewmh_atoms[NET_WM_WINDOW_TYPE_DESKTOP] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
	ewmh_atoms[NET_WM_WINDOW_TYPE_DOCK] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	ewmh_atoms[NET_WM_ALLOWED_ACTIONS] = XInternAtom(dpy, "_NET_WM_ALLOWED_ACTIONS", False);
	ewmh_atoms[NET_WM_ACTION_MINIMIZE] = XInternAtom(dpy, "_NET_WM_ACTION_MINIMIZE", False);
	ewmh_atoms[NET_WM_ACTION_CLOSE] = XInternAtom(dpy, "_NET_WM_ACTION_CLOSE", False);
	ewmh_atoms[NET_WM_ACTION_MAXIMIZE_HORZ] = XInternAtom(dpy, "_NET_WM_ACTION_MAXIMIZE_HORZ", False);
	ewmh_atoms[NET_WM_ACTION_MAXIMIZE_VERT] = XInternAtom(dpy, "_NET_WM_ACTION_MAXIMIZE_VERT", False);
	ewmh_atoms[NET_WM_ACTION_FULLSCREEN] = XInternAtom(dpy, "_NET_WM_ACTION_FULLSCREEN", False);
	ewmh_atoms[NET_WM_ACTION_MOVE] = XInternAtom(dpy, "_NET_WM_ACTION_MOVE", False);
	ewmh_atoms[NET_WM_ACTION_RESIZE] = XInternAtom(dpy, "_NET_WM_ACTION_RESIZE", False);
	ewmh_atoms[NET_WM_ACTION_ABOVE] = XInternAtom(dpy, "_NET_WM_ACTION_ABOVE", False);
	ewmh_atoms[NET_WM_ACTION_BELOW] = XInternAtom(dpy, "_NET_WM_ACTION_BELOW", False);
	ewmh_atoms[NET_WM_ACTION_CHANGE_DESKTOP] = XInternAtom(dpy, "_NET_WM_ACTION_CHANGE_DESKTOP", False);
	ewmh_atoms[NET_WM_MOVERESIZE] = XInternAtom(dpy, "_NET_WM_MOVERESIZE", False);
	ewmh_atoms[NET_RESTACK_WINDOW] = XInternAtom(dpy, "_NET_RESTACK_WINDOW", False);
	ewmh_atoms[NET_FRAME_EXTENTS] = XInternAtom(dpy, "_NET_FRAME_EXTENTS", False);
	ewmh_atoms[NET_REQUEST_FRAME_EXTENTS] = XInternAtom(dpy, "_NET_REQUEST_FRAME_EXTENTS", False);
	ewmh_atoms[NET_SHOWING_DESKTOP] = XInternAtom(dpy, "_NET_SHOWING_DESKTOP", False);
	ewmh_atoms[KDE_NET_WM_WINDOW_TYPE_OVERRIDE] = XInternAtom(dpy, "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE", False);
	if(XGetWindowProperty(dpy, root, ewmh_atoms[NET_CURRENT_DESKTOP], 0, 1, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &p) == Success) {
		if(nir) {
			d = *(long *) p; /* if we would just pass &d to XGetWindowProperty we break strict aliasing rules */
			if(d >= dc)
				d = dc - 1;
			if(d < 0)
				d = 0;
			desktop_goto(d);
		}
		XFree((void *) p);
	}
}

void ewmh_update(void) {
	long vp[] = {0, 0};
	XChangeProperty(dpy, root, ewmh_atoms[NET_SUPPORTED], XA_ATOM, 32, PropModeReplace, (unsigned char *) &ewmh_atoms, EWMH_ATOM_COUNT);
	XChangeProperty(dpy, root, ewmh_atoms[NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wlist, 1);
	XChangeProperty(dpy, wlist, ewmh_atoms[NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wlist, 1);
	XChangeProperty(dpy, wlist, ewmh_atoms[NET_WM_NAME], XA_STRING, 8, PropModeReplace, (unsigned char *) NAME, strlen(NAME));
	XChangeProperty(dpy, root, ewmh_atoms[NET_DESKTOP_VIEWPORT], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &vp, 2);
	XDeleteProperty(dpy, root, ewmh_atoms[NET_DESKTOP_NAMES]);
	ewmh_update_number_of_desktops();
	ewmh_set_desktop(desktop);
	ewmh_update_geometry();
	ewmh_update_showing_desktop();
}

bool ewmh_handle_event(XEvent *ev) {
	client *c;
	int i, j, xo, yo;
	long extents[4];
	Atom rt;
	int rf;
	unsigned long nir, bar;
	switch(ev->type) {
		case ClientMessage:
			c = owner(ev->xclient.window);
			if(ev->xclient.message_type == ewmh_atoms[NET_WM_MOVERESIZE]) {
				if(c) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_WM_MOVERESIZE\n");
					#endif
					if(ev->xclient.data.l[2] == NET_WM_MOVERESIZE_MOVE || ev->xclient.data.l[2] == NET_WM_MOVERESIZE_MOVE_KEYBOARD) {
						client_focus(c, true);
						xo = client_width_total_intern(c) / 2;
						yo = client_height_total_intern(c) / 2;
						XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, xo, yo);
						drag_start(A_MOVE, AnyButton, client_x(c) + xo, client_y(c) + yo);
					}
					if(ev->xclient.data.l[2] == NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT || ev->xclient.data.l[2] == NET_WM_MOVERESIZE_SIZE_KEYBOARD) {
						client_focus(c, true);
						drag_start(A_RESIZE, AnyButton, ev->xclient.data.l[0], ev->xclient.data.l[1]);
					}
				}
				return true;
			}
			if(ev->xclient.message_type == ewmh_atoms[NET_CLOSE_WINDOW]) {
				if(c) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_CLOSE_WINDOW\n");
					#endif
					delete_window(c);
				}
				return true;
			}
			if(ev->xclient.message_type == ewmh_atoms[NET_ACTIVE_WINDOW]) {
				if(c) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_ACTIVE_WINDOW\n");
					#endif
					if(c->flags & ICONIC)
						client_restore(c);
					else {
						if(c->desktop != desktop && c->desktop != STICKY)
							desktop_goto(c->desktop);
						client_raise(c);
					}
					if(evh != wlist_handle_event)
						client_focus(c, true);
				}
				return true;
			}
			if(ev->xclient.message_type == ewmh_atoms[NET_RESTACK_WINDOW]) {
				if(c && ev->xclient.data.l[1] == None) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_RESTACK_WINDOW\n");
					#endif
					client_raise(c);
				}
				/* schould also add code for handling this when a sibling window is passed */
				/* but we schould find/create a way to test this first */
				return true;
			}
			if(ev->xclient.message_type == ewmh_atoms[NET_WM_STATE]) {
				if(c) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_WM_STATE\n");
					#endif
					j = 0;
					for(i = 1; i < 3; i++) {
						if(((Atom) ev->xclient.data.l[i]) == ewmh_atoms[NET_WM_STATE_MAXIMIZED_HORZ])
							j |= MAXIMIZED_L | MAXIMIZED_R;
						if(((Atom) ev->xclient.data.l[i]) == ewmh_atoms[NET_WM_STATE_MAXIMIZED_VERT])
							j |= MAXIMIZED_T | MAXIMIZED_B;
					}
					if(j)
						client_toggle_state(c, j);
					if(((Atom) ev->xclient.data.l[1]) == ewmh_atoms[NET_WM_STATE_FULLSCREEN] && (ev->xclient.data.l[0] == NET_WM_STATE_TOGGLE || (ev->xclient.data.l[0] == NET_WM_STATE_ADD && !(c->flags & FULLSCREEN)) || (ev->xclient.data.l[0] == NET_WM_STATE_REMOVE && (c->flags & FULLSCREEN))))
						client_fullscreen(c);
					if(((Atom) ev->xclient.data.l[1]) == ewmh_atoms[NET_WM_STATE_ABOVE]) {
						if(((Atom) ev->xclient.data.l[0]) == ewmh_atoms[NET_WM_STATE_ADD])
							client_set_layer(c, TOP);
						if(((Atom) ev->xclient.data.l[0]) == ewmh_atoms[NET_WM_STATE_REMOVE] && c->layer == TOP)
							client_set_layer(c, NORMAL);
						if(((Atom) ev->xclient.data.l[0]) == ewmh_atoms[NET_WM_STATE_TOGGLE])
							client_set_layer(c, (c->layer == TOP) ? NORMAL : TOP);
					}
					if(((Atom) ev->xclient.data.l[1]) == ewmh_atoms[NET_WM_STATE_BELOW]) {
						if(((Atom) ev->xclient.data.l[0]) == ewmh_atoms[NET_WM_STATE_ADD])
							client_set_layer(c, BOTTOM);
						if(((Atom) ev->xclient.data.l[0]) == ewmh_atoms[NET_WM_STATE_REMOVE] && c->layer == BOTTOM)
							client_set_layer(c, NORMAL);
						if(((Atom) ev->xclient.data.l[0]) == ewmh_atoms[NET_WM_STATE_TOGGLE])
							client_set_layer(c, (c->layer == BOTTOM) ? NORMAL : BOTTOM);
					}
					return true;
				}
			}
			if(ev->xclient.message_type == ewmh_atoms[NET_CURRENT_DESKTOP]) {
				#ifdef DEBUG_EVENTS
				printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_CURRENT_DESKTOP\n");
				#endif
				desktop_goto(ev->xclient.data.l[0]);
				return true;
			}
			if(ev->xclient.message_type == ewmh_atoms[NET_WM_DESKTOP]) {
				if(c && ev->xclient.data.l[0] >= STICKY) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_WM_DESKTOP\n");
					#endif
					client_to_desktop(c, (ev->xclient.data.l[0] <= dc) ? ev->xclient.data.l[0] : dc - 1);
				}
				return true;
			}
			if(ev->xclient.message_type == ewmh_atoms[NET_REQUEST_FRAME_EXTENTS]) {
				#ifdef DEBUG_EVENTS
				printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_REQUEST_FRAME_EXTENTS\n");
				#endif
				extents[0] = border_width;
				extents[1] = border_width;
				extents[2] = border_width + title_height;
				extents[3] = border_width;
				XChangeProperty(dpy, ev->xclient.window, ewmh_atoms[NET_FRAME_EXTENTS], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &extents, 4);	
				return true;
			}
			if(ev->xclient.message_type == ewmh_atoms[NET_SHOWING_DESKTOP]) {
				#ifdef DEBUG_EVENTS
				printf(NAME ": ewmh_handle_event(): handling ClientMessage event\n\tatom: _NET_SHOWING_DESKTOP\n");
				#endif
				client_iconify_all();
				return true;
			}
			break;
		case PropertyNotify:
			if(ev->xproperty.atom == ewmh_atoms[NET_WM_STRUT_PARTIAL] || ev->xproperty.atom == ewmh_atoms[NET_WM_STRUT]) {
				#ifdef DEBUG_EVENTS
				printf(NAME ": ewmh_handle_event(): handling PropertyNotify event (_NET_WM_STRUT or _NET_WM_STRUT_PARTIAL changed)\n");
				#endif
				ewmh_update_strut();
				return true;
			}
			c = owner(ev->xproperty.window);
			if(c && ev->xproperty.atom == ewmh_atoms[NET_WM_NAME]) {
				if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_NAME], 0, 1024, False, xa_utf8_string, &rt, &rf, &nir, &bar, (unsigned char **) &c->ewmh_name) != Success)
					c->ewmh_name = NULL;
				else client_update_name(c);
				return true;
			}
			break;
		}
	return false;
}

void ewmh_get_hints(client *c) {
	Atom rt, data;
	int rf;
	unsigned long nir, bar;
	unsigned char *p;
	long d;
	if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_DESKTOP], 0, 1, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &p) == Success) {
		if(nir) {
			d = *(long *) p; /* if we would just pass &d to XGetWindowProperty we break strict aliasing rules */
			if(d < STICKY)
				c->desktop = 0;
			else if(d >= dc)
				c->desktop = dc - 1;
			else c->desktop = d;
		}
		XFree((void *) p);
	}
	if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_STATE], 0, 1, False, XA_ATOM, &rt, &rf, &nir, &bar, (unsigned char **) &p) == Success) {
		if(nir) {
			data = *(Atom *) p; /* if we would just pass &data to XGetWindowProperty we break strict aliasing rules */
			if(data == ewmh_atoms[NET_WM_STATE_FULLSCREEN])
				c->flags |= FULLSCREEN;
			if(data == ewmh_atoms[NET_WM_STATE_ABOVE])
				c->layer = TOP;
			if(data == ewmh_atoms[NET_WM_STATE_BELOW])
				c->layer = BOTTOM;
		}
		XFree((void *) p);
	}
	if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_WINDOW_TYPE], 0, 1, False, XA_ATOM, &rt, &rf, &nir, &bar, (unsigned char **) &p) == Success) {
		while(nir) {
			data = *(Atom *) p; /* if we would just pass &data to XGetWindowProperty we break strict aliasing rules */
			if(data == ewmh_atoms[NET_WM_WINDOW_TYPE_DESKTOP]) {
				c->flags ^= c->flags & (CAN_MOVE | CAN_RESIZE | HAS_BORDER | HAS_TITLE);
				c->flags |= NO_STRUT | DONT_LIST | FULLSCREEN | CLICK_FOCUS | DESKTOP_LOCKED;
				c->layer = DESKTOP;
				c->desktop = STICKY;
				c->screen = ewmh_screen;
			}
			if(data == ewmh_atoms[NET_WM_WINDOW_TYPE_DOCK]) {
				c->flags ^= c->flags & (CAN_MOVE | CAN_RESIZE | HAS_BORDER | HAS_TITLE);
				c->flags |= NO_STRUT | DONT_LIST | DONT_FOCUS | DESKTOP_LOCKED | IS_TASKBAR;
				c->desktop = STICKY;
				c->screen = ewmh_screen;
				if(taskbar_ontop)
					c->layer = TOP;
			}
			if(data == ewmh_atoms[KDE_NET_WM_WINDOW_TYPE_OVERRIDE])
				c->flags |= DONT_LIST | NO_STRUT;
			data++;
			nir--;
		}
		XFree((void *) p);
	}
	if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_NAME], 0, 1024, False, xa_utf8_string, &rt, &rf, &nir, &bar, (unsigned char **) &c->ewmh_name) != Success)
		c->ewmh_name = NULL;
}

void ewmh_update_extents(client *c) {
	long extents[4];
	extents[0] = client_border(c);
	extents[1] = client_border(c);
	extents[2] = client_border(c) + client_title(c);
	extents[3] = client_border(c);
	XChangeProperty(dpy, c->window, ewmh_atoms[NET_FRAME_EXTENTS], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &extents, 4);
}

void ewmh_update_geometry(void) {
	long ds[2];
	int scr = (ewmh_screen < nscreens) ? ewmh_screen : nscreens - 1;
	ds[0] = screens[scr].width - screens[scr].x;
	ds[1] = screens[scr].height - screens[scr].y;
	XChangeProperty(dpy, root, ewmh_atoms[NET_DESKTOP_GEOMETRY], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &ds, 2);
}

void ewmh_update_number_of_desktops(void) {
	XChangeProperty(dpy, root, ewmh_atoms[NET_NUMBER_OF_DESKTOPS], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &dc, 1);
}

void ewmh_update_desktop(client *c) {
	XChangeProperty(dpy, c->window, ewmh_atoms[NET_WM_DESKTOP], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &c->desktop, 1);
}

void ewmh_set_desktop(int d) {
	XChangeProperty(dpy, root, ewmh_atoms[NET_CURRENT_DESKTOP], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &d, 1);
}

void ewmh_set_active(client *c) {
	Window aw = c ? c->window : None;
	XChangeProperty(dpy, root, ewmh_atoms[NET_ACTIVE_WINDOW], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &aw, 1);
}

void ewmh_update_allowed_actions(client *c) {
	int nactions = 3;
	Atom actions[12];
	actions[0] = ewmh_atoms[NET_WM_ACTION_CLOSE];
	actions[1] = ewmh_atoms[NET_WM_ACTION_ABOVE];
	actions[2] = ewmh_atoms[NET_WM_ACTION_BELOW];
	if(c->flags & CAN_MOVE && c->flags & CAN_RESIZE) {
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MAXIMIZE_HORZ];
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MAXIMIZE_VERT];
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_FULLSCREEN];
	}
	if(c->flags & CAN_MOVE)
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MOVE];
	if(c->flags & CAN_RESIZE)
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_RESIZE];
	if(!(c->flags & DONT_LIST))
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MINIMIZE];
	if(!(c->flags & DESKTOP_LOCKED))
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_CHANGE_DESKTOP];
	XChangeProperty(dpy, c->window, ewmh_atoms[NET_WM_ALLOWED_ACTIONS], XA_ATOM, 32, PropModeReplace, (unsigned char *) &actions, nactions);
}

void ewmh_update_state(client *c) {
	int statec = 0;
	Atom state[12];
	if(c->flags & MAXIMIZED_L && c->flags & MAXIMIZED_R)
		state[statec++] = ewmh_atoms[NET_WM_STATE_MAXIMIZED_HORZ];
	if(c->flags & MAXIMIZED_T && c->flags & MAXIMIZED_B)
		state[statec++] = ewmh_atoms[NET_WM_STATE_MAXIMIZED_VERT];
	if(c->flags & FULLSCREEN)
		state[statec++] = ewmh_atoms[NET_WM_STATE_FULLSCREEN];
	if(client_layer(c) <= TOP)
		state[statec++] = ewmh_atoms[NET_WM_STATE_ABOVE];
	if(client_layer(c) >= BOTTOM)
		state[statec++] = ewmh_atoms[NET_WM_STATE_BELOW];
	XChangeProperty(dpy, c->window, ewmh_atoms[NET_WM_STATE], XA_ATOM, 32, PropModeReplace, (unsigned char *) &state, statec);
}

void ewmh_update_stacking(void) {
	int i, n = 0;
	Window *data = _malloc(cn * sizeof(Window));
	for(i = cn - 1; i >= 0; i--)
		if(!(stacking[i]->flags & DONT_LIST))
			data[n++] = stacking[i]->window;
	XChangeProperty(dpy, root, ewmh_atoms[NET_CLIENT_LIST_STACKING], XA_WINDOW, 32, PropModeReplace, (unsigned char *) data, n);
	free(data);
}

void ewmh_update_clist(void) {
	int i, n = 0;
	Window *data = _malloc(cn * sizeof(Window));
	for(i = cn - 1; i >= 0; i--)
		if(!(clients[i]->flags & DONT_LIST))
			data[n++] = clients[i]->window;
	XChangeProperty(dpy, root, ewmh_atoms[NET_CLIENT_LIST], XA_WINDOW, 32, PropModeReplace, (unsigned char *) data, n);
	free(data);
	ewmh_update_stacking();
	ewmh_update_strut();
}

void ewmh_update_strut(void) {
	Atom rt;
	int i, j, rf, scr;
	unsigned long nir, bar;
	long workarea[16], *data;
	unsigned char *p;
	for(i = 0; i < nscreens; i++)
		for(j = 0; j < 4; j++)
			screens[i].ewmh_strut[j] = 0;
	for(i = 0; i < cn; i++) {
		client_update_screen(clients[i]);
		if(XGetWindowProperty(dpy, clients[i]->window, ewmh_atoms[NET_WM_STRUT_PARTIAL], 0, 4, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &p) != Success || nir < 4)
			if(XGetWindowProperty(dpy, clients[i]->window, ewmh_atoms[NET_WM_STRUT], 0, 4, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &p) != Success || nir < 4)
				continue;
		data = (long *) p;
		if(data[0])
			screens[clients[i]->screen].ewmh_strut[0] += data[0] - (screens_leftmost() + screens[clients[i]->screen].x);
		if(data[1])
			screens[clients[i]->screen].ewmh_strut[1] += data[1] - (screens_rightmost() - (screens[clients[i]->screen].x + screens[clients[i]->screen].width));
		if(data[2])
			screens[clients[i]->screen].ewmh_strut[2] += data[2] - (screens_topmost() + screens[clients[i]->screen].y);
		if(data[3])
			screens[clients[i]->screen].ewmh_strut[3] += data[3] - (screens_bottom() - (screens[clients[i]->screen].y + screens[clients[i]->screen].height));
		XFree((void *) p);
	}
	/* This is the old code, i have no idea what i was thinking back then.
	scr = (ewmh_screen < nscreens) ? ewmh_screen : nscreens - 1;
	workarea[0] = screens[scr].x + screens[scr].ewmh_strut[0];
	workarea[1] = screens[scr].y + screens[scr].ewmh_strut[2];
	workarea[2] = (screens[scr].width - screens[scr].x) - (screens[scr].ewmh_strut[0] + screens[scr].ewmh_strut[1]);
	workarea[3] = (screens[scr].height - screens[scr].y) - (screens[scr].ewmh_strut[2] + screens[scr].ewmh_strut[3]);
	*/
	
	workarea[2] = 0;
	workarea[3] = 0;
	for (i = 0; i < nscreens; ++i) {
		int x = screens[i].x + screens[i].ewmh_strut[0];
		int y = screens[i].y + screens[i].ewmh_strut[2];
		workarea[0] = ((workarea[0] < x) ? workarea[0] : x);
		workarea[1] = ((workarea[1] < y) ? workarea[1] : y);
		workarea[2] += screens[i].width - (screens[i].ewmh_strut[0] + screens[i].ewmh_strut[1]);
		workarea[3] += screens[i].height - (screens[i].ewmh_strut[2] + screens[i].ewmh_strut[3]);
	}
	//printf("%d %d %d %d", screens[scr].x, screens[scr].y, screens[scr].width, screens[scr].height);
	
	workarea[4] = workarea[0]; /* why 4 times? ask gnome developpers, this is the only way nautilus will listen to it */
	workarea[5] = workarea[1];
	workarea[6] = workarea[2];
	workarea[7] = workarea[3];
	workarea[8] = workarea[0];
	workarea[9] = workarea[1];
	workarea[10] = workarea[2];
	workarea[11] = workarea[3];
	workarea[12] = workarea[0];
	workarea[13] = workarea[1];
	workarea[14] = workarea[2];
	workarea[15] = workarea[3];
	XChangeProperty(dpy, root, ewmh_atoms[NET_WORKAREA], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &workarea, sizeof(workarea) / sizeof(long));
	for(i = 0; i < cn; i++)
		client_update(clients[i]);
}

void ewmh_update_showing_desktop(void) {
	XChangeProperty(dpy, root, ewmh_atoms[NET_SHOWING_DESKTOP], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &all_iconic, 1);
}
