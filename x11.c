#include "matwm.h"

int xerrorstatus = 0;

int xerrorhandler(Display *display, XErrorEvent *xerror) { /* we set this as the X error handler in main() */
	#ifdef DEBUG
	client *c;
	#endif
	if(xerror->error_code == BadAccess && xerror->resourceid == root)
		xerrorstatus++;
	#ifdef DEBUG
	else {
		char ret[666];
		XGetErrorText(xerror->display, xerror->error_code, ret, sizeof(ret));
		c = owner(xerror->resourceid);
		printf(NAME ": x error: %s\n\tresource: 0x%X (%s)\n\topcode: %i (%s)\n", ret, (unsigned int) xerror->resourceid, c ? c->name : ((xerror->resourceid == root) ? "root" : "unknown"), xerror->request_code, str_opcode(xerror->request_code));
	}
	#endif
	return 0;
}

bool select_root_events(void) {
	/* make sure xerrorstatus is 0 */
	XSync(dpy, False);
	xerrorstatus = 0;
	/* select events and check if an error occurred */
	XSelectInput(dpy, root, StructureNotifyMask | SubstructureRedirectMask | SubstructureNotifyMask | FocusChangeMask | (click_root ? ButtonPressMask : 0)); /* some of this events can only be selected by one application, this will error if another window manager is running */
	XSync(dpy, False); /* calls the error handler if any errors occured */
	if(xerrorstatus) { /* if XSelectInput() has failed, this will be set */
		fprintf(stderr, NAME ": error: can't select events on the root window\n");
		return false;
	}
	return true;
}

void get_normal_hints(client *c) { /* read normal size hints */
	long sr;
	if(!XGetWMNormalHints(dpy, c->window, &c->normal_hints, &sr))
		c->normal_hints.flags = 0;
	if(c->normal_hints.flags & PMinSize && c->normal_hints.flags & PMaxSize && c->normal_hints.min_width == c->normal_hints.max_width && c->normal_hints.min_height == c->normal_hints.max_height)
		c->flags ^= c->flags & CAN_RESIZE; /* min and max size are equal, so the window can't be resized */
}

void get_wm_hints(client *c, int *state_hint) { /* read hint for the initial state of a window */
	XWMHints *wm_hints = XGetWMHints(dpy, c->window);
	*state_hint = WithdrawnState;
	if(wm_hints) {
		if(wm_hints->flags & StateHint)
			*state_hint = wm_hints->initial_state;
		c->want_input_focus = (wm_hints->flags & InputHint && wm_hints->input == False) ? false : true;
		XFree(wm_hints);
	}
}

int get_wm_state(Window w) { /* read current state of the window */
	Atom rt;
	int rf;
	unsigned long n, bar;
	unsigned char *data;
	long ret = WithdrawnState;
	if(XGetWindowProperty(dpy, w, xa_wm_state, 0, 1, False, AnyPropertyType, &rt, &rf, &n, &bar, &data) == Success && n) {
		ret = *(long *) data;
		XFree(data);
	}
	return ret;
}

void set_wm_state(Window w, long state) { /* set WM_STATE property */
	long data[2];
	data[0] = (long) state;
	data[1] = None;
	XChangeProperty(dpy, w, xa_wm_state, xa_wm_state, 32, PropModeReplace, (unsigned char *) data, 2);
}

void get_mwm_hints(client *c) { /* read motif hints */
	Atom rt;
	int rf;
	unsigned long nir, bar;
	unsigned char *p;
	MWMHints *mwmhints;
	if(XGetWindowProperty(dpy, c->window, xa_motif_wm_hints, 0, 3, False, AnyPropertyType, &rt, &rf, &nir, &bar, (unsigned char **) &p) == Success) {
		if(nir > 2) {
			mwmhints = (MWMHints *) p; /* schould we pass &mwmhints directly to XGetWindowProperty, we break strict aliasing rules */
			if(mwmhints->flags & MWM_HINTS_FUNCTIONS) {
				c->flags ^= c->flags & (CAN_MOVE | CAN_RESIZE);
				if(mwmhints->functions & MWM_FUNC_ALL) /* this means reverse all bits */
					mwmhints->functions = (MWM_FUNC_MOVE | MWM_FUNC_RESIZE) & (~mwmhints->functions);
				if(mwmhints->functions & MWM_FUNC_MOVE)
					c->flags |= CAN_MOVE;
				if(mwmhints->functions & MWM_FUNC_RESIZE)
					c->flags |= CAN_RESIZE;
			}
			if(mwmhints->flags & MWM_HINTS_DECORATIONS) {
				c->flags ^= c->flags & (HAS_TITLE | HAS_BORDER);
				if(mwmhints->decorations & MWM_DECOR_ALL) /* equivalent of MWM_FUNC_ALL */
					mwmhints->decorations = (MWM_DECOR_TITLE | MWM_DECOR_BORDER | MWM_DECOR_RESIZEH) & (~mwmhints->decorations);
				if(mwmhints->decorations & MWM_DECOR_TITLE)
					c->flags |= HAS_TITLE;
				if(mwmhints->decorations & MWM_DECOR_BORDER)
					c->flags |= HAS_BORDER;
				if(!(mwmhints->decorations & MWM_DECOR_RESIZEH))
					c->flags ^= CAN_RESIZE & c->flags; /* remove CAN_RESIZE if it has been set */
			}
		}
		XFree((void *) p);
	}
}

#ifdef USE_SHAPE
void set_shape(client *c) { /* make the parent window of c have the same shape as its client window */
	if(c->flags & SHAPED)
		XShapeCombineShape(dpy, c->parent, ShapeBounding, client_border(c), client_border(c) + client_title(c), c->window, ShapeBounding, ShapeSet);
}
#endif

void configurenotify(client *c) { /* informs a client about its geometry */
	XConfigureEvent ce;
	ce.type = ConfigureNotify;
	ce.event = c->window;
	ce.window = c->window;
	ce.x = client_x(c) + client_border(c);
	ce.y = client_y(c) + client_border(c) + client_title(c);
	ce.width = client_width(c);
	ce.height = client_height(c);
	ce.border_width = 0;
	ce.above = None;
	ce.override_redirect = 0;
	XSendEvent(dpy, c->window, False, StructureNotifyMask, (XEvent *) &ce);
}

int has_protocol(Window w, Atom protocol) { /* to check if a window supports specified protocol - used below */
	int i, count, ret = 0;
	Atom *protocols;
	if(XGetWMProtocols(dpy, w, &protocols, &count)) {
		for(i = 0; i < count; i++)
			if(protocols[i] == protocol)
				ret++;
		XFree(protocols);
	}
	return ret;
}

void delete_window(client *c) { /* for closing windows */
	XEvent ev;
	if(has_protocol(c->window, xa_wm_delete)) { /* check if the window supports WM_DELETE, wich is more friendly then just killing the client */
		ev.type = ClientMessage;
		ev.xclient.window = c->window;
		ev.xclient.message_type = xa_wm_protocols;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = xa_wm_delete;
		ev.xclient.data.l[1] = CurrentTime; /* */
		XSendEvent(dpy, c->window, False, NoEventMask, &ev);
	} else XKillClient(dpy, c->window);
}

void take_focus(client *c) {
	XEvent ev;
	if(c->want_input_focus)
		XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
	else XSetInputFocus(dpy, None, RevertToPointerRoot, CurrentTime);
	if(has_protocol(c->window, xa_wm_take_focus)) {
		ev.type = ClientMessage;
		ev.xclient.window = c->window;
		ev.xclient.message_type = xa_wm_protocols;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = xa_wm_take_focus;
		ev.xclient.data.l[1] = CurrentTime; /* ICCCM says we schould use a valid timestamp instead, poke me if this breaks something */
		XSendEvent(dpy, c->window, False, NoEventMask, &ev);
	}
}

int gxo(client *c, bool initial) { /* returns offset for horizontal window gravity */
	if(c->normal_hints.flags & PWinGravity)
		switch(c->normal_hints.win_gravity) {
			case StaticGravity:
				return client_border(c);
			case NorthGravity:
			case SouthGravity:
			case CenterGravity:
				return client_border(c) + (initial ? -c->oldbw : (c->width / 2));
			case NorthEastGravity:
			case EastGravity:
			case SouthEastGravity:
				return ((client_border(c) * 2) + (initial ? -(c->oldbw * 2) : c->width)) + ((c->flags & NO_STRUT) ? 0 : screens[c->screen].ewmh_strut[1]);
		}
	return ((c->flags & NO_STRUT) ? 0 : -screens[c->screen].ewmh_strut[0]);
}

int gyo(client *c, bool initial) { /* returns offset for vertical window gravity */
	if(c->normal_hints.flags & PWinGravity)
		switch(c->normal_hints.win_gravity) {
			case StaticGravity:
				return client_border(c) + client_title(c);
			case EastGravity:
			case WestGravity:
			case CenterGravity:
				return client_border(c) + ((client_title(c) + (initial ? -c->oldbw : c->height)) / 2);
			case SouthEastGravity:
			case SouthGravity:
			case SouthWestGravity:
				return ((client_border(c) * 2) + client_title(c) + (initial ? -(c->oldbw * 2) : c->height)) + ((c->flags & NO_STRUT) ? 0 : screens[c->screen].ewmh_strut[3]);
		}
	return ((c->flags & NO_STRUT) ? 0 : -screens[c->screen].ewmh_strut[2]);
}

void window_correct_center(Window w) {
	XWindowAttributes attr;
	XGetWindowAttributes(dpy, w, &attr);
	if(screens_correct_center(&attr.x, &attr.y, &attr.width, &attr.height))
		XMoveWindow(dpy, w, attr.x, attr.y);
}

bool has_child(Window parent, Window child) { /* checks if child is a child of parent */
	unsigned int i, nwins;
	Window dw, *wins;
	XQueryTree(dpy, parent, &dw, &dw, &wins, &nwins);
	for(i = 0; i < nwins; i++)
		if(wins[i] == child)
			return true;
	return false;
}

Window get_focus_window(void) {
	Window ret;
	int revert_to;
	XGetInputFocus(dpy, &ret, &revert_to);
	return ret;
}

int isviewable(Window w) { /* check if a window is actually viewable */
	XWindowAttributes attr;
	if(XGetWindowAttributes(dpy, w, &attr) != 0)
		if(attr.map_state == IsViewable)
			return 1;
	return 0;
}

Bool isunmap(Display *display, XEvent *event, XPointer arg) { /* predicate procedure to look for an UnmapNotify for a specific window */
	if(event->type == UnmapNotify && event->xunmap.window == *(Window *) arg)
		return True;
	return False;
}
