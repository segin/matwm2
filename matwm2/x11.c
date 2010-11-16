#include "matwm.h"

int xerrorhandler(Display *display, XErrorEvent *xerror) {
	if(xerror->error_code == BadAccess && xerror->resourceid == root) {
		fprintf(stderr,"error: root window at display %s is not available\n", XDisplayName(dn));
		exit(1);
	}
#ifdef DEBUG
	else {
		char ret[666];
		XGetErrorText(xerror->display, xerror->error_code, ret, 666);
		client *c = owner(xerror->resourceid);
		if(c) printf("%i (%s): x error: %s\n", xerror->resourceid, c->name, ret);
		else printf("%i: x error: %s\n", xerror->resourceid, ret);
	}
#endif
	return 0;
}

void get_normal_hints(client *c) {
	long sr;
	XGetWMNormalHints(dpy, c->window, &c->normal_hints, &sr);
	if(c->normal_hints.flags & PMinSize && c->normal_hints.flags & PMaxSize && c->normal_hints.min_width == c->normal_hints.max_width && c->normal_hints.min_height == c->normal_hints.max_height)
		c->flags ^= c->flags & CAN_RESIZE;
}

int get_state_hint(Window w) {
	int ret = WithdrawnState;
	XWMHints *wm_hints = XGetWMHints(dpy, w);
	if(wm_hints) {
		if(wm_hints->flags & StateHint)
			ret = wm_hints->initial_state;
		XFree(wm_hints);
	}
	return ret;
}

int get_wm_state(Window w) {
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

void set_wm_state(Window w, long state) {
	long data[2];
	data[0] = (long) state;
	data[1] = None;
	XChangeProperty(dpy, w, xa_wm_state, xa_wm_state, 32, PropModeReplace, (unsigned char *) data, 2);
}

void get_mwm_hints(client *c) {
	Atom rt;
	int rf;
	unsigned long nir, bar;
	MWMHints *mwmhints;
	if(XGetWindowProperty(dpy, c->window, xa_motif_wm_hints, 0, 3, False, AnyPropertyType, &rt, &rf, &nir, &bar, (unsigned char **) &mwmhints) == Success) {
		if(nir > 2) {
			if(mwmhints->flags & MWM_HINTS_DECORATIONS) {
				c->flags ^= c->flags & (HAS_TITLE | HAS_BORDER | CAN_MOVE | CAN_RESIZE);
				if(mwmhints->functions & MWM_FUNC_ALL) {
					mwmhints->functions &= ~MWM_FUNC_ALL;
					mwmhints->functions = (MWM_FUNC_RESIZE | MWM_FUNC_MOVE) & (~mwmhints->functions);
				}
				if(mwmhints->decorations & MWM_DECOR_ALL) {
					mwmhints->decorations &= ~MWM_DECOR_ALL;
					mwmhints->decorations = (MWM_DECOR_TITLE | MWM_DECOR_BORDER | MWM_DECOR_RESIZEH) & (~mwmhints->decorations);
				}
				if(mwmhints->functions & MWM_FUNC_MOVE)
					c->flags |= CAN_MOVE;
				if(mwmhints->functions & MWM_FUNC_RESIZE)
					c->flags |= CAN_RESIZE;
				if(mwmhints->decorations & MWM_DECOR_TITLE)
					c->flags |= HAS_TITLE;
				if(mwmhints->decorations & MWM_DECOR_BORDER)
					c->flags |= HAS_BORDER;
			}
		}
		XFree((void *) mwmhints);
	}
}

#ifdef SHAPE
void set_shape(client *c) {
	if(c->flags & SHAPED)
		XShapeCombineShape(dpy, c->parent, ShapeBounding, client_border(c), client_border(c) + client_title(c), c->window, ShapeBounding, ShapeSet);
}
#endif

void configurenotify(client *c)
{
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

int has_protocol(Window w, Atom protocol) {
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

void delete_window(client *c) {
	XEvent ev;
	if(has_protocol(c->window, xa_wm_delete)) {
		ev.type = ClientMessage;
		ev.xclient.window = c->window;
		ev.xclient.message_type = xa_wm_protocols;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = xa_wm_delete;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, c->window, False, NoEventMask, &ev);
	} else XKillClient(dpy, c->window);
}

int gxo(client *c, int initial) {
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
				return ((client_border(c) * 2) + (initial ? -(c->oldbw * 2) : c->width)) + ((c->flags & NO_STRUT) ? 0 : ewmh_strut[1]);
		}
	return ((c->flags & NO_STRUT) ? 0 : -ewmh_strut[0]);
}

int gyo(client *c, int initial) {
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
				return ((client_border(c) * 2) + client_title(c) + (initial ? -(c->oldbw * 2) : c->height)) + ((c->flags & NO_STRUT) ? 0 : ewmh_strut[3]);
		}
	return ((c->flags & NO_STRUT) ? 0 : -ewmh_strut[2]);
}

int has_child(Window parent, Window child) {
	unsigned int i, nwins;
	Window dw, *wins;
	XQueryTree(dpy, parent, &dw, &dw, &wins, &nwins);
	for(i = 0; i < nwins; i++)
		if(wins[i] == child)
			return 1;
	return 0;
}

int isviewable(Window w) {
	XWindowAttributes attr;
	XGetWindowAttributes(dpy, w, &attr);
	if(attr.map_state == IsViewable)
		return 1;
	return 0;
}

Bool isunmap(Display *display, XEvent *event, XPointer arg) {
	if(event->type == UnmapNotify && event->xunmap.window == *(Window *) arg)
		return True;
	return False;
}

