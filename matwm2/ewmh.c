#include "matwm.h"

Atom ewmh_atoms[EWMH_ATOM_COUNT];
long ewmh_strut[4];

void ewmh_initialize(void) {
	Atom rt;
	int rf;
	long nir, bar, *d;
	long vp[] = {0, 0};
	ewmh_atoms[NET_SUPPORTED] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	ewmh_atoms[NET_SUPPORTING_WM_CHECK] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	ewmh_atoms[NET_WM_NAME] =	XInternAtom(dpy, "_NET_WM_NAME", False);
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
	ewmh_atoms[NET_FRAME_EXTENTS] = XInternAtom(dpy, "_NET_FRAME_EXTENTS", False);
	ewmh_atoms[NET_REQUEST_FRAME_EXTENTS] = XInternAtom(dpy, "_NET_REQUEST_FRAME_EXTENTS", False);
	ewmh_atoms[NET_SHOWING_DESKTOP] = XInternAtom(dpy, "_NET_SHOWING_DESKTOP", False);
	XChangeProperty(dpy, root, ewmh_atoms[NET_SUPPORTED], XA_ATOM, 32, PropModeReplace, (unsigned char *) &ewmh_atoms, EWMH_ATOM_COUNT);
	XChangeProperty(dpy, root, ewmh_atoms[NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wlist, 1);
	XChangeProperty(dpy, wlist, ewmh_atoms[NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wlist, 1);
	XChangeProperty(dpy, wlist, ewmh_atoms[NET_WM_NAME], XA_STRING, 8, PropModeReplace, (unsigned char *) NAME, strlen(NAME));
	XChangeProperty(dpy, root, ewmh_atoms[NET_DESKTOP_VIEWPORT], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &vp, 2);
	XChangeProperty(dpy, root, ewmh_atoms[NET_NUMBER_OF_DESKTOPS], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &dc, 1);
	XDeleteProperty(dpy, root, ewmh_atoms[NET_DESKTOP_NAMES]);
	if(XGetWindowProperty(dpy, root, ewmh_atoms[NET_CURRENT_DESKTOP], 0, 1, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &d) == Success) {
		if(nir) {
			if(*d >= dc)
				*d = dc - 1;
			if(*d < 0)
				*d = 0;
			desktop_goto(*d);
		}
		XFree((void *) d);
	}
	ewmh_set_desktop(desktop);
	ewmh_update_geometry();
	ewmh_update_showing_desktop();
}

int ewmh_handle_event(XEvent ev) {
	client *c;
	int i;
	switch(ev.type) {
		case ClientMessage:
			c = owner(ev.xclient.window);
			if(c && ev.xclient.message_type == ewmh_atoms[NET_WM_MOVERESIZE]) {
				if(ev.xclient.data.l[2] == NET_WM_MOVERESIZE_MOVE || ev.xclient.data.l[2] == NET_WM_MOVERESIZE_MOVE_KEYBOARD) {
					client_focus(c);
					int xo = client_width_total_intern(c) / 2, yo = client_height_total_intern(c) / 2;
					XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, xo, yo);
					drag_start(MOVE, AnyButton, client_x(c) + xo, client_y(c) + yo);
				}
				if(ev.xclient.data.l[2] == NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT || ev.xclient.data.l[2] == NET_WM_MOVERESIZE_SIZE_KEYBOARD) {
					client_focus(c);
					drag_start(RESIZE, AnyButton, ev.xclient.data.l[0], ev.xclient.data.l[1]);
				}
			}
			if(ev.xclient.message_type == ewmh_atoms[NET_CLOSE_WINDOW]) {
				if(c)
					delete_window(c);
				return 1;
			}
			if(ev.xclient.message_type == ewmh_atoms[NET_ACTIVE_WINDOW]) {
				if(c) {
					if(c->flags & ICONIC) {
						client_restore(c);
						client_focus(c);
					} else {
						if(c->desktop != desktop && c->desktop != STICKY)
							desktop_goto(c->desktop);
						client_raise(c);
						client_focus(c);
					}
				}
				return 1;
			}
			if(c && ev.xclient.message_type == ewmh_atoms[NET_WM_STATE]) {
				int s = 0;
				for(i = 1; i < 3; i++) {
					if(ev.xclient.data.l[i] == ewmh_atoms[NET_WM_STATE_MAXIMIZED_HORZ])
						s |= MAXIMISED_L | MAXIMISED_R;
					if(ev.xclient.data.l[i] == ewmh_atoms[NET_WM_STATE_MAXIMIZED_VERT])
						s |= MAXIMISED_T | MAXIMISED_B;
				}
				if(s)
					client_toggle_state(c, s);
				if(ev.xclient.data.l[1] == ewmh_atoms[NET_WM_STATE_FULLSCREEN] && (ev.xclient.data.l[0] == NET_WM_STATE_TOGGLE || (ev.xclient.data.l[0] == NET_WM_STATE_ADD && !(c->flags & FULLSCREEN)) || (ev.xclient.data.l[0] == NET_WM_STATE_REMOVE && (c->flags & FULLSCREEN))))
					client_toggle_state(c, FULLSCREEN);
				if(ev.xclient.data.l[1] == ewmh_atoms[NET_WM_STATE_ABOVE])
					client_set_layer(c, (c->layer == TOP) ? NORMAL : TOP);
				if(ev.xclient.data.l[1] == ewmh_atoms[NET_WM_STATE_BELOW])
					client_set_layer(c, (c->layer == BOTTOM) ? NORMAL : BOTTOM);
				return 1;
			}
			if(ev.xclient.message_type == ewmh_atoms[NET_CURRENT_DESKTOP])
				desktop_goto(ev.xclient.data.l[0]);
			if(c && ev.xclient.message_type == ewmh_atoms[NET_WM_DESKTOP])
				if(ev.xclient.data.l[0] >= STICKY)
					client_to_desktop(c, (ev.xclient.data.l[0] <= dc) ? ev.xclient.data.l[0] : dc - 1);
			if(ev.xclient.message_type == ewmh_atoms[NET_REQUEST_FRAME_EXTENTS]) {
				long e[] = {border_width, border_width, border_width + title_height, border_width};
				XChangeProperty(dpy, ev.xclient.window, ewmh_atoms[NET_FRAME_EXTENTS], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &e, 4);	
			}
			if(ev.xclient.message_type == ewmh_atoms[NET_SHOWING_DESKTOP])
				client_iconify_all();
			break;
		case PropertyNotify:
			if(ev.xproperty.atom == ewmh_atoms[NET_WM_STRUT_PARTIAL] || ev.xproperty.atom == ewmh_atoms[NET_WM_STRUT]) {
				ewmh_update_strut();
				return 1;
			}
			break;
		}
	return 0;
}

int ewmh_get_hints(client *c) {
	Atom rt, *data;
	int rf;
	long nir, bar, *d;
	if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_DESKTOP], 0, 1, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &d) == Success) {
		if(nir) {
			if(*d < STICKY)
				c->desktop = 0;
			else if(*d >= dc)
				c->desktop = dc - 1;
			else c->desktop = *d;
		}
		XFree((void *) d);
	}
	if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_STATE], 0, 1, False, XA_ATOM, &rt, &rf, &nir, &bar, (unsigned char **) &data) == Success) {
		if(nir) {
			if(*data == ewmh_atoms[NET_WM_STATE_FULLSCREEN])
				c->flags |= FULLSCREEN;
			if(*data == ewmh_atoms[NET_WM_STATE_ABOVE])
				c->layer = TOP;
			if(*data == ewmh_atoms[NET_WM_STATE_BELOW])
				c->layer = BOTTOM;
		}
		XFree((void *) data);
	}
	if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_WINDOW_TYPE], 0, 1, False, XA_ATOM, &rt, &rf, &nir, &bar, (unsigned char **) &data) == Success) {
		if(nir) {
			if(*data == ewmh_atoms[NET_WM_WINDOW_TYPE_DESKTOP]) {
				c->flags ^= c->flags & (CAN_MOVE | CAN_RESIZE | HAS_BORDER | HAS_TITLE);
				c->flags |= NO_STRUT | DONT_LIST | FULLSCREEN | DONT_FOCUS;
				c->layer = DESKTOP;
				c->desktop = STICKY;
			}
			if(*data == ewmh_atoms[NET_WM_WINDOW_TYPE_DOCK]) {
				c->flags ^= c->flags & (CAN_MOVE | CAN_RESIZE | HAS_BORDER | HAS_TITLE);
				c->flags |= NO_STRUT | DONT_LIST | DONT_FOCUS;
				c->desktop = STICKY;
			}
		}
		XFree((void *) data);
	}
}

void ewmh_update_extents(client *c) {
	long e[] = {client_border(c), client_border(c), client_border(c) + client_title(c), client_border(c)};
	XChangeProperty(dpy, c->window, ewmh_atoms[NET_FRAME_EXTENTS], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &e, 4);
}

void ewmh_update_geometry(void) {
	long ds[] = {display_width, display_height};
	XChangeProperty(dpy, root, ewmh_atoms[NET_DESKTOP_GEOMETRY], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &ds, 2);
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
	int nactions = 5;
	Atom actions[12] = {ewmh_atoms[NET_WM_ACTION_MINIMIZE], ewmh_atoms[NET_WM_ACTION_CLOSE], ewmh_atoms[NET_WM_ACTION_CHANGE_DESKTOP], ewmh_atoms[NET_WM_ACTION_ABOVE], ewmh_atoms[NET_WM_ACTION_BELOW]};
	if(c->flags & CAN_MOVE && c->flags & CAN_RESIZE) {
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MAXIMIZE_HORZ];
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MAXIMIZE_VERT];
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_FULLSCREEN];
	}
	if(c->flags & CAN_MOVE)
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MOVE];
	if(c->flags & CAN_RESIZE)
		actions[nactions++] = ewmh_atoms[NET_WM_ACTION_RESIZE];
	XChangeProperty(dpy, c->window, ewmh_atoms[NET_WM_ALLOWED_ACTIONS], XA_ATOM, 32, PropModeReplace, (unsigned char *) &actions, nactions);
}

void ewmh_update_state(client *c) {
	int statec = 0;
	Atom state[12];
	if(c->flags & MAXIMISED_L && c->flags & MAXIMISED_R)
		state[statec++] = ewmh_atoms[NET_WM_STATE_MAXIMIZED_HORZ];
	if(c->flags & MAXIMISED_T && c->flags & MAXIMISED_B)
		state[statec++] = ewmh_atoms[NET_WM_STATE_MAXIMIZED_VERT];
	if(c->flags & FULLSCREEN)
		state[statec++] = ewmh_atoms[NET_WM_STATE_FULLSCREEN];
	if(c->layer == TOP)
		state[statec++] = ewmh_atoms[NET_WM_STATE_ABOVE];
	if(c->layer == BOTTOM)
		state[statec++] = ewmh_atoms[NET_WM_STATE_BELOW];
	XChangeProperty(dpy, c->window, ewmh_atoms[NET_WM_STATE], XA_ATOM, 32, PropModeReplace, (unsigned char *) &state, statec);
}

void ewmh_update_stacking(void) {
	int i, n = 0;
	Window data[cn];
	for(i = cn - 1; i >= 0; i--)
		if(!(stacking[i]->flags & DONT_LIST))
			data[n++] = stacking[i]->window;
	XChangeProperty(dpy, root, ewmh_atoms[NET_CLIENT_LIST_STACKING], XA_WINDOW, 32, PropModeReplace, (unsigned char *) data, n);
}

void ewmh_update_clist(void) {
	int i, n = 0;
	Window data[cn];
	for(i = cn - 1; i >= 0; i--)
		if(!(clients[i]->flags & DONT_LIST))
			data[n++] = clients[i]->window;
	XChangeProperty(dpy, root, ewmh_atoms[NET_CLIENT_LIST], XA_WINDOW, 32, PropModeReplace, (unsigned char *) data, n);
	ewmh_update_stacking();
	ewmh_update_strut();
}

void ewmh_update_strut(void) {
	Atom rt;
	int i, rf;
	unsigned long nir, bar;
	long workarea[4], *data;
	for(i = 0; i < 4; i++)
		ewmh_strut[i] = 0;
	for(i = 0; i < cn; i++) {
		if(XGetWindowProperty(dpy, clients[i]->window, ewmh_atoms[NET_WM_STRUT_PARTIAL], 0, 4, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &data) != Success || nir < 4)
			if(XGetWindowProperty(dpy, clients[i]->window, ewmh_atoms[NET_WM_STRUT], 0, 4, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &data) != Success || nir < 4)
				continue;
		ewmh_strut[0] += data[0];
		ewmh_strut[1] += data[1];
		ewmh_strut[2] += data[2];
		ewmh_strut[3] += data[3];
		XFree((void *) data);
	}
	workarea[0] = ewmh_strut[0];
	workarea[1] = ewmh_strut[2];
	workarea[2] = display_width - (ewmh_strut[0] + ewmh_strut[1]);
	workarea[3] = display_height - (ewmh_strut[2] + ewmh_strut[3]);
	XChangeProperty(dpy, root, ewmh_atoms[NET_WORKAREA], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &workarea, 4);
}

void ewmh_update_showing_desktop(void) {
	XChangeProperty(dpy, root, ewmh_atoms[NET_SHOWING_DESKTOP], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &all_iconic, 1);
}

