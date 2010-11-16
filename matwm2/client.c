#include "matwm.h"

client **clients = NULL, **stacking = NULL, *current = NULL, *previous = NULL;
int cn = 0, nicons = 0;

void client_add(Window w) {
	XWindowAttributes attr;
	int i, di, bounding_shaped, wm_state = get_wm_state(w);
	unsigned int dui;
	client *new = (client *) malloc(sizeof(client));
	wm_state = get_wm_state(w);
	XGetWindowAttributes(dpy, w, &attr);
	if(wm_state == WithdrawnState) {
		wm_state = get_state_hint(w);
		set_wm_state(w, wm_state != WithdrawnState ? wm_state : NormalState);
	}
	new->width = attr.width;
	new->height = attr.height;
	new->oldbw = attr.border_width;
	new->window = w;
	new->flags = HAS_TITLE | HAS_BORDER | HAS_BUTTONS | CAN_MOVE | CAN_RESIZE;
	new->layer = NORMAL;
	new->desktop = desktop;
#ifdef SHAPE
	if(have_shape && XShapeQueryExtents(dpy, new->window, &bounding_shaped, &di, &di, &dui, &dui, &di, &di, &di, &dui, &dui) && bounding_shaped)
		new->flags |= SHAPED;
#endif
	get_normal_hints(new);
	get_mwm_hints(new);
	ewmh_get_hints(new);
	new->xo = gxo(new, 1);
	new->yo = gyo(new, 1);
	new->x = attr.x - new->xo;
	new->y = attr.y - new->yo;
	if(wm_state == IconicState)
		new->flags |= ICONIC;
	XSelectInput(dpy, w, PropertyChangeMask | FocusChangeMask);
#ifdef SHAPE
	XShapeSelectInput(dpy, w, ShapeNotifyMask);
#endif
	XSetWindowBorderWidth(dpy, w, 0);
	new->parent = XCreateWindow(dpy, root, client_x(new), client_y(new), client_width_total_intern(new), client_height_total_intern(new), (new->flags & HAS_BORDER) ? 1 : 0,
	                            DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                            CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWEventMask, &p_attr);
	new->title = XCreateWindow(dpy, new->parent, border_width - 1, border_width - 1, 1, 1, 0,
	                           DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                           CWOverrideRedirect | CWEventMask, &p_attr);
	new->wlist_item = XCreateWindow(dpy, wlist, 0, 0, 1, 1, 0,
	                                DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
	                                CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
	XFetchName(dpy, w, &new->name);
	buttons_create(new);
	client_update_name(new);
	client_grab_buttons(new);
	XMapWindow(dpy, new->title);
	if(!(new->flags & DONT_LIST))
		XMapWindow(dpy, new->wlist_item);
	XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, new->parent, client_border_intern(new), client_border_intern(new) + client_title(new));
	if(new->flags & FULLSCREEN || new->flags & MAXIMIZED_L || new->flags & MAXIMIZED_R || new->flags & MAXIMIZED_T || new->flags & MAXIMIZED_B)
		client_update_size(new);
	else
		configurenotify(new);
#ifdef SHAPE
	set_shape(new);
#endif
	cn++;
	clients_alloc();
	if(!(new->flags & ICONIC)) {
		for(i = cn - 1; i > 0 && (stacking[i - 1]->flags & ICONIC || client_layer(stacking[i - 1]) >= client_layer(new)); i--)
			stacking[i] = stacking[i - 1];
		stacking[i] = new;
		if(evh == drag_handle_event)
			client_raise(current);
		if(new->desktop == desktop || new->desktop == STICKY)
			client_show(new);
	} else {
		stacking[cn - 1] = new;
		nicons++;
	}
	clients[cn - 1] = new;
	if(evh == wlist_handle_event)
		wlist_update();
	else if((focus_new || !current) && !(new->flags & ICONIC))
		client_focus(new);
	ewmh_update_desktop(new);
	ewmh_update_allowed_actions(new);
	ewmh_update_state(new);
	ewmh_update_extents(new);
	ewmh_update_clist();
}

void client_show(client *c) {
	XMapWindow(dpy, c->window);
	XMapWindow(dpy, c->parent);
	clients_apply_stacking();
}

void client_hide(client *c) {
	XEvent ev;
	XUnmapWindow(dpy, c->parent);
	XUnmapWindow(dpy, c->window);
	XIfEvent(dpy, &ev, &isunmap, (XPointer) &c->window);
	if(c == previous)
		previous = NULL;
	if(c == current) {
		if(evh == drag_handle_event)
			evh = drag_release_wait;
		client_focus(NULL);
	}
}

void client_deparent(client *c) {
	XReparentWindow(dpy, c->window, root, client_x(c) + c->xo, client_y(c) + c->yo);
	XSetWindowBorderWidth(dpy, c->window, c->oldbw);
	XRemoveFromSaveSet(dpy, c->window);
}

void client_remove(client *c) {
	XEvent ev;
	int i;
	if(c->flags & ICONIC)
		nicons--;
	for(i = 0; i < c->nbuttons; i++)
		if(button_current == &c->buttons[i])
			button_current = NULL;
	XDestroyWindow(dpy, c->parent);
	XDestroyWindow(dpy, c->wlist_item);
	free((void *) c->buttons);
	if(c->name != no_title)
		XFree(c->name);
	XFreePixmap(dpy, c->title_pixmap);
	for(i = client_number(clients, c) + 1; i < cn; i++)
		clients[i - 1] = clients[i];
	for(i = client_number(stacking, c) + 1; i < cn; i++)
		stacking[i - 1] = stacking[i];
	cn--;
	if(c == previous)
		previous = NULL;
	if(c == current) {
		current = NULL;
		client_focus_first();
	}
	free(c);
	clients_alloc();
	if(evh == wlist_handle_event)
		wlist_update();
	ewmh_update_clist();
}

void client_grab_button(client *c, int button) {
	if(!(c->flags & DONT_FOCUS)) {
		button_grab(c->parent, button, mousemodmask, ButtonPressMask | ButtonReleaseMask);
		if(nosnapmodmask && (buttonaction(button, 0) == BA_MOVE || buttonaction(button, 0) == BA_RESIZE || buttonaction(button, 1) == BA_MOVE || buttonaction(button, 1) == BA_RESIZE))
			button_grab(c->parent, button, nosnapmodmask | mousemodmask, ButtonPressMask | ButtonReleaseMask);
	}
}

void client_grab_buttons(client *c) {
	if(mousemodmask) {
		client_grab_button(c, Button1);
		client_grab_button(c, Button2);
		client_grab_button(c, Button3);
		client_grab_button(c, Button4);
		client_grab_button(c, Button5);
	}
	XGrabButton(dpy, AnyButton, AnyModifier, c->window, True, ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
}

void client_draw_title(client *c) {
	XFillRectangle(dpy, c->title_pixmap, (c == current) ? bgc : ibgc, 0, 0, c->title_width, text_height);
	XDrawString(dpy, c->title_pixmap, (c == current) ? gc : igc, 0, font->max_bounds.ascent, c->name, strlen(c->name));
}

void client_update_name(client *c) {
	if(!c->name || strlen(c->name) == 0)
		c->name = no_title;
	c->title_width = XTextWidth(font, c->name, strlen(c->name)) + 1;
	c->title_pixmap = XCreatePixmap(dpy, c->title, c->title_width, text_height, DefaultDepth(dpy, screen));
	client_draw_title(c);
	XSetWindowBackgroundPixmap(dpy, c->title, c->title_pixmap);
	XMoveResizeWindow(dpy, c->title, client_title_y(c), border_width - 1, client_title_width(c), text_height);
}

void client_set_bg(client *c, XColor color, XColor border) {
	int i;
	XSetWindowBackground(dpy, c->parent, color.pixel);
	client_draw_title(c);
	XSetWindowBorder(dpy, c->parent, border.pixel);
	XSetWindowBackground(dpy, c->button_parent_left, color.pixel);
	XSetWindowBackground(dpy, c->button_parent_right, color.pixel);
	for(i = 0; i < c->nbuttons; i++)
		XSetWindowBackground(dpy, c->buttons[i].w, color.pixel);
	XSetWindowBackground(dpy, c->wlist_item, color.pixel);
	if(c->desktop == desktop || c->desktop == STICKY) {
		XClearWindow(dpy, c->parent);
		XClearWindow(dpy, c->title);
		XClearWindow(dpy, c->button_parent_left);
		XClearWindow(dpy, c->button_parent_right);
		buttons_draw(c);
		client_draw_title(c);
	}
	if(evh == wlist_handle_event) {
		XClearWindow(dpy, c->wlist_item);
		wlist_item_draw(c);
	}
}

void clients_apply_stacking(void) {
	int i = 0;
	Window wins[cn + 1];
	wins[0] = wlist;
	for(i = 0; i < cn && !(stacking[i]->flags & ICONIC); i++)
		wins[i + 1] = stacking[i]->parent;
	XRestackWindows(dpy, wins, i + 1);
	ewmh_update_stacking();
	if(evh == wlist_handle_event)
		wlist_update();
}

void client_update_pos(client *c) {
	XMoveWindow(dpy, c->parent, client_x(c), client_y(c));
	configurenotify(c);
}

void client_update_size(client *c) {
	int width = client_width(c);
	buttons_update(c);
	XMoveResizeWindow(dpy, c->title, client_title_y(c), border_width - 1, client_title_width(c), text_height);
	XResizeWindow(dpy, c->parent, client_width_total_intern(c), client_height_total_intern(c));
	XResizeWindow(dpy, c->window, width, client_height(c));
	buttons_draw(c);
}

void client_update(client *c) {
	int width = client_width(c);
	buttons_update(c);
	XMoveResizeWindow(dpy, c->title, client_title_y(c), border_width - 1, client_title_width(c), text_height);
	XMoveResizeWindow(dpy, c->parent, client_x(c), client_y(c), client_width_total_intern(c), client_height_total_intern(c));
	XMoveResizeWindow(dpy, c->window, client_border_intern(c), client_border_intern(c) + client_title(c), width, client_height(c));
	buttons_draw(c);
}

void client_update_title(client *c) {
	XMoveWindow(dpy, c->window, client_border_intern(c), client_border_intern(c) + client_title(c));
	client_update_size(c);
	ewmh_update_extents(c);
}

void client_update_layer(client *c, int prev) {
	int i;
	if(client_layer(c) > prev)
		for(i = client_number(stacking, c); i < cn - 1 && client_layer(stacking[i + 1]) < client_layer(c) && !(stacking[i + 1]->flags & ICONIC); i++)
			stacking[i] = stacking[i + 1];
	else
		for(i = client_number(stacking, c); i > 0 && client_layer(stacking[i - 1]) > client_layer(c); i--)
			stacking[i] = stacking[i - 1];
	stacking[i] = c;
	clients_apply_stacking();
	ewmh_update_state(c);
}

void client_warp(client *c) {
	XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, client_width_total_intern(c) - 1, client_height_total_intern(c) - 1);
}

void client_focus_first(void) {
	int i;
	if(previous && (previous->desktop == desktop || previous->desktop == STICKY)) {
		client_focus(previous);
		return;
	}
	for(i = 0; i < cn; i++)
		if(client_visible(stacking[i]) && !(stacking[i]->flags & DONT_FOCUS)) {
			client_focus(stacking[i]);
			break;
		}
	if(i == cn)
		client_focus(NULL);
}

void client_clear_state(client *c) {
	c->x = client_x(c);
	c->y = client_y(c);
	c->width = client_width(c);
	c->height = client_height(c);
	if(c->flags & (MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B | EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B | FULLSCREEN)) {
		c->flags ^= c->flags & (MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B | EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B | FULLSCREEN);
		ewmh_update_state(c);
	}
}

void clients_alloc(void) {
	if(!cn) {
		free(clients);
		free(stacking);
		clients = NULL;
		stacking = NULL;
		return;
	}
	client **newptr = (client **) realloc((void *) clients, cn * sizeof(client *));
	if(!newptr)
		error();
	clients = newptr;
	newptr = (client **) realloc((void *) stacking, cn * sizeof(client *));
	if(!newptr)
		error();
	stacking = newptr;
}

