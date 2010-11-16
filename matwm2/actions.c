#include "matwm.h"

int all_iconic = 0;

void client_move(client *c, int x, int y) {
	if(x == client_x(c) && y == client_y(c))
		return;
	client_clear_state(c);
	c->x = x;
	c->y = y;
	client_update_pos(c);
	return;
}

void client_resize(client *c, int width, int height) {
	if(c->normal_hints.flags & PResizeInc) {
		width -= (width - ((c->normal_hints.flags & PBaseSize) ? c->normal_hints.base_width : 0)) % c->normal_hints.width_inc;
		height -= (height - ((c->normal_hints.flags & PBaseSize) ? c->normal_hints.base_height : 0)) % c->normal_hints.height_inc;
	}
	if(c->normal_hints.flags & PAspect) {
		if(height < (width *	c->normal_hints.min_aspect.y) / c->normal_hints.min_aspect.x)
			height = (width * c->normal_hints.min_aspect.y) / c->normal_hints.min_aspect.x;
		if(height > (width *	c->normal_hints.max_aspect.y) / c->normal_hints.max_aspect.x)
			height = (width * c->normal_hints.max_aspect.y) / c->normal_hints.max_aspect.x;
	}
	if(c->normal_hints.flags & PMinSize) {
		if(width < c->normal_hints.min_width)
			width = c->normal_hints.min_width;
		if(height < c->normal_hints.min_height)
			height = c->normal_hints.min_height;
	}
	if(c->normal_hints.flags & PMaxSize) {
		if(width > c->normal_hints.max_width)
			width = c->normal_hints.max_width;
		if(height > c->normal_hints.max_height)
			height = c->normal_hints.max_height;
	}
	if(width < MINSIZE)
		width = MINSIZE;
	if(height < MINSIZE)
		height = MINSIZE;
	if(width == client_width(c) && height == client_height(c))
		return;
	client_clear_state(c);
	c->width = width;
	c->height = height;
	client_update_size(c);
	return;
}

void client_focus(client *c, bool set_input_focus) {
	client *prev = current;
	if(c)
		if(c->flags & DONT_FOCUS)
			return;
	if(evh == drag_handle_event)
		return;
	if(current)
		if(!(current->flags & ICONIC))
			previous = current;
	current = c;
	if(prev)
		client_set_bg(prev, ibg, ibfg);
	if(c) {
		client_set_bg(c, bg, bfg);
		if((c->desktop == desktop || c->desktop == STICKY) && isviewable(c->window) && evh != wlist_handle_event && set_input_focus) {
			take_focus(c);
		}
	} else if(set_input_focus)
		XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	ewmh_set_active(c);
}

void client_raise(client *c) {
	int i;
	for(i = client_number(stacking, c); i > 0 && (client_layer(stacking[i - 1]) >= client_layer(c) || stacking[i - 1]->flags & ICONIC || (fullscreen_stacking == FS_ONTOP && c->flags & FULLSCREEN && c->layer <= NORMAL)); i--)
		stacking[i] = stacking[i - 1];
	stacking[i] = c;
	client_over_fullscreen(c); /* fullscreen windows can go above always-on-top windows, but we can also go above them with the help of this function */
	/* above function calls clients_apply_stacking(), so we are done */
}

void client_lower(client *c) {
	int i;
	for(i = client_number(stacking, c); i < cn - 1 && client_layer(stacking[i + 1]) <= client_layer(c) && !(stacking[i + 1]->flags & ICONIC); i++)
		stacking[i] = stacking[i + 1];
	stacking[i] = c;
	clients_apply_stacking();
}

void client_fullscreen(client *c) {
	int prev = client_layer(current);
	client_toggle_state(c, FULLSCREEN);
	if(fullscreen_stacking == FS_ALWAYS_ONTOP)
	  client_update_layer(current, prev);
	if(fullscreen_stacking == FS_ONTOP && !(c->flags & FULLSCREEN))
	  client_update_layer(current, TOP);
}

void client_set_layer(client *c, int layer) { /* for making windows always-on-top, always-below, etc */
	int i, prev = client_layer(c);
	c->layer = layer;
	if(client_layer(c) != prev)
		client_update_layer(c, prev);
	for(i = 0; i < c->nbuttons; i++)
		if(c->buttons[i].action == B_ONTOP || c->buttons[i].action == B_BELOW)
			button_draw(c, &c->buttons[i]);
}

void client_toggle_state(client *c, int state) { /* used directly only for maximizing, client_fullscreen also calls this  */
	if(!(c->flags & CAN_MOVE) || !(c->flags & CAN_RESIZE))
		return;
	client_update_screen(c);
	if((c->flags & state) == state)
		c->flags ^= state;
	else {
		c->flags |= state;
		client_raise(c);
	}
	client_update(c);
	ewmh_update_state(c);
}

void client_expand_x(client *c, int d, int first) { /* calculate horizontal dimensions for expanding */
	int i, right, client_right;
	client_right = c->x + c->width + ((c->flags & HAS_BORDER) ? (border_width + border_spacing) * 2 : 0); /* we don't any of the functions from info.c because they report dimensions specific to the state of the window */
	for(i = 0; i < cn; i++) {
		if(!client_visible(clients[i]) || (first ? c->y : c->expand_y) >= client_y(clients[i]) + client_height_total(clients[i]) || (first ? c->y + client_height_total(c) : c->expand_height) <= client_y(clients[i]))
			continue;
		if(d & EXPANDED_L && client_x(clients[i]) + client_width_total(clients[i]) <= c->x && client_x(clients[i]) + client_width_total(clients[i]) > c->expand_x)
			c->expand_x = client_x(clients[i]) + client_width_total(clients[i]);
		if(d & EXPANDED_R && client_x(clients[i]) >= client_right && client_x(clients[i]) < c->expand_width)
			c->expand_width = client_x(clients[i]);
	}
	for(i = 0; i < nscreens; i++) {
		if((first ? c->y : c->expand_y) >= screens[i].y + screens[i].height || (first ? c->y + client_height_total(c) : c->expand_height) <= screens[i].y)
			continue;
		if(d & EXPANDED_L && screens[i].x <= c->x && screens[i].x > c->expand_x)
			c->expand_x = screens[i].x;
		if(d & EXPANDED_R && screens[i].x >= client_right && screens[i].x < c->expand_width) /* we also snap to the outsides of screens, because this might be useful with overlapping screens */
			c->expand_width = screens[i].x;
		right = screens[i].x + screens[i].width;
		if(d & EXPANDED_L && right <= c->x && right > c->expand_x)
			c->expand_x = right;
		if(d & EXPANDED_R && right >= client_right && right < c->expand_width)
			c->expand_width = right;
	}
}

void client_expand_y(client *c, int d, int first) { /* calculate horizontal dimensions for expanding */
	int i, bottom, client_bottom;
	client_bottom = c->y + c->height + ((c->flags & HAS_BORDER) ? (border_width + border_spacing) * 2 : 0) + ((c->flags & HAS_TITLE) ? title_height : 0);
	for(i = 0; i < cn; i++) {
		if(!client_visible(clients[i]) || (first ? c->x : c->expand_x) >= client_x(clients[i]) + client_width_total(clients[i]) || (first ? c->x + client_width_total(c) : c->expand_width) <= client_x(clients[i]))
			continue;
		if(d & EXPANDED_T && client_y(clients[i]) + client_height_total(clients[i]) <= c->y && client_y(clients[i]) + client_height_total(clients[i]) > c->expand_y)
			c->expand_y = client_y(clients[i]) + client_height_total(clients[i]);
		if(d & EXPANDED_B && client_y(clients[i]) >= client_bottom && client_y(clients[i]) < c->expand_height)
			c->expand_height = client_y(clients[i]);
	}
	for(i = 0; i < nscreens; i++) {
		if((first ? c->x : c->expand_x) >= screens[i].x + screens[i].width || (first ? c->x + client_width_total(c) : c->expand_width) <= screens[i].x)
			continue;
		if(d & EXPANDED_T && screens[i].y <= c->y && screens[i].y > c->expand_y)
			c->expand_y = screens[i].y;
		if(d & EXPANDED_B && screens[i].y >= client_bottom && screens[i].y < c->expand_height) /* we also snap to the outsides of screens, because this might be useful with overlapping screens */
			c->expand_height = screens[i].y;
		bottom = screens[i].y + screens[i].height;
		if(d & EXPANDED_T && bottom <= c->y && bottom > c->expand_y)
			c->expand_y = bottom;
		if(d & EXPANDED_B && bottom >= client_bottom && bottom < c->expand_height)
			c->expand_height = bottom;
	}
}

void client_expand(client *c, int d, bool a) { /* see expand key action in the manual page for explanation of what this does */
	int borders = ((c->flags & HAS_BORDER) ? (border_width + border_spacing) * 2 : 0), borders_and_title = borders + ((c->flags & HAS_TITLE) ? title_height : 0); /* don't use client_border() etc, they depend on state */
	if(!(c->flags & CAN_MOVE) || !(c->flags & CAN_RESIZE))
		return;
	if((c->flags & d) == d) {
		c->flags ^= c->flags & d;
		client_update(c);
		return;
	}
	d ^= (d & c->flags);
	client_update_screen(c);
	c->expand_x = (d & EXPANDED_L) ? screens[c->screen].x : c->x;
	c->expand_y = (d & EXPANDED_T) ? screens[c->screen].y : c->y;
	c->expand_width = (d & EXPANDED_R) ? screens[c->screen].x + screens[c->screen].width : (c->x + c->width + borders);
	c->expand_height = (d & EXPANDED_B) ? screens[c->screen].y + screens[c->screen].height : (c->y + c->height + borders_and_title);
	if(a) {
		client_expand_y(c, d, true);
		client_expand_x(c, d, false);
	} else {
		client_expand_x(c, d, true);
		client_expand_y(c, d, false);
	}
	c->expand_width -= c->expand_x + borders;
	c->expand_height -= c->expand_y + borders_and_title;
	c->flags |= d;
	client_update(c);
}

void client_toggle_title(client *c) { /* hide or unhide title bar */
	c->flags ^= HAS_TITLE;
	client_update_title(c);
}

void client_iconify(client *c) {
	int i;
	XEvent ev;
	if(c->flags & ICONIC || c->flags & DONT_LIST)
		return;
	nicons++;
	c->flags |= ICONIC;
	set_wm_state(c->window, IconicState);
	if(c->desktop == desktop || c->desktop == STICKY) {
		client_hide(c);
		XUnmapWindow(dpy, c->window); /* window needs to be unmapped so it can send maprequest to de-iconify itself */
		XIfEvent(dpy, &ev, &isunmap, (XPointer) &c->window); /* catch the UnmapNotify event so matwm doesn't think the client is gone */
	}
	if(!current)
		client_focus_first();
	if(c->desktop != desktop && c->desktop != STICKY)
		XMapWindow(dpy, c->wlist_item);
	for(i = client_number(stacking, c); i < cn - 1; i++)
		stacking[i] = stacking[i + 1];
	stacking[cn - 1] = c;
	ewmh_update_stacking();
	if(evh == wlist_handle_event)
		wlist_update();
}

void client_restore(client *c) { /* restores iconic client */
	if(!(c->flags & ICONIC))
		return;
	nicons--;
	XMapWindow(dpy, c->window);
	client_show(c);
	if(c->desktop != STICKY && c->desktop != desktop) {
		c->desktop = desktop;
		ewmh_update_desktop(c);
	}
	c->flags ^=ICONIC;
	client_raise(c);
	set_wm_state(c->window, NormalState);
	if(c == current)
		XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
	if(evh == wlist_handle_event)
		wlist_update();
	client_end_all_iconic();
}

void client_save(client *c) { /* bring a moved off-screen window back to the screen */
	bool lost = true;
	int i, x = client_x(c), y = client_y(c);
	int right = x + client_width_total(c), bottom = client_height_total(c);
	for(i = 0; i < nscreens; i++)
		if(x < screens[i].x + screens[i].width && y < screens[i].y + screens[i].height && right > screens[i].x && bottom > screens[i].y)
			lost = false;
	if(lost) {
		screens_update_current();
		if(map_center)
			client_move(c, screens[cs].x + (screens[cs].width / 2) - (client_width(c) / 2), screens[cs].y + (screens[cs].height / 2) - (client_height(c) / 2));
		else client_move(c, screens[cs].x, screens[cs].y);
	}
}

void client_to_border(client *c, char *a) {
	int x = client_x(c), y = client_y(c);
	if(!(c->flags & CAN_MOVE))
		return;
	client_update_screen(c);
	while(a && *a) {
		if(*a == 'l')
			x = screens[c->screen].x + screens[c->screen].ewmh_strut[0];
		if(*a == 'r')
			x = (screens[c->screen].x + screens[c->screen].width) - (client_width_total(c) + screens[c->screen].ewmh_strut[1]);
		if(*a == 't')
			y = screens[c->screen].y + screens[c->screen].ewmh_strut[2];
		if(*a == 'b')
			y = (screens[c->screen].y + screens[c->screen].height) - (client_height_total(c) + screens[c->screen].ewmh_strut[3]);
		a++;
	}
	client_move(c, x, y);
	client_raise(c);
	client_warp(c);
}

void client_iconify_all(void) { /* iconify all windows, to be eventually restored afterwards by calling this function again */
	int i;
	if(all_iconic) {
		for(i = cn - 1; i >= 0; i--)
			if(stacking[i]->flags & RESTORE) {
				stacking[i]->flags ^= RESTORE;
				client_restore(stacking[i]);
				i++;
			}
		all_iconic = 0;
	} else {
		for(i = 0; i < cn; i++)
			stacking[i]->flags ^= stacking[i]->flags & RESTORE;
		for(i = 0; i < cn; i++)
			if(client_visible(stacking[i]) && !(stacking[i]->flags & DONT_LIST)) {
				stacking[i]->flags |= RESTORE;
				client_iconify(stacking[i]);
				i--;
			}
		all_iconic = 1;
	}
	ewmh_update_showing_desktop();
}

void client_end_all_iconic(void) { /* to exit state induced by above function without restoring all windows */
	if(all_iconic) {
		all_iconic = 0;
		ewmh_update_showing_desktop();
	}
}

void client_warp(client *c) { /* moves the pointer to a client */
	XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, client_width_total_intern(c) - 1, client_height_total_intern(c) - 1);
}

void client_focus_first(void) { /* to be called when focus window is lost */
	int i;
	unsigned int ui;
	Window w, dw;
	client *c = NULL;
	XEvent ev;
	/* check if the pointer just dropped into a window */
	XSync(dpy, False);
	while(XCheckTypedEvent(dpy, EnterNotify, &ev))
		c = owner(ev.xcrossing.window);
	if(c) {
		client_focus(c, true);
		return;
	}
	/* check if we can focus the previously focussed window */
	if(previous && client_visible(previous) && !(previous->flags & DONT_FOCUS)) {
		client_focus(previous, true);
		return;
	}
	/* check if the pointer is in a window */
	XQueryPointer(dpy, root, &dw, &w, &i, &i, &i, &i, &ui);
	c = owner(w);
	if(c && !(c->flags & DONT_FOCUS)) {
		client_focus(c, true);
		return;
	}
	/* try to focus the window on top of the stack */
	for(i = 0; i < cn; i++)
		if(client_visible(stacking[i]) && !(stacking[i]->flags & DONT_FOCUS)) {
			client_focus(stacking[i], true);
			return;
		}
	/* if we still found no window, give up and focus NULL */
	if(current)
		client_focus(NULL, true);
}

void client_action(client *c, action *act, XEvent *ev) {
	char *a;
	int i = 0, j = 0;
	if(!act) /* this happens */
		return;
	a = act->arg;
	if(current)
		switch(act->code) {
			case A_ICONIFY:
				client_iconify(c);
				return;
			case A_MAXIMIZE:
				while(a && *a) {
					if(*a == 'h')
						i |= MAXIMIZED_L | MAXIMIZED_R;
					if(*a == 'v')
						i |= MAXIMIZED_T | MAXIMIZED_B;
					if(*a == 'l')
						i |= MAXIMIZED_L;
					if(*a == 'r')
						i |= MAXIMIZED_R;
					if(*a == 'u')
						i |= MAXIMIZED_T;
					if(*a == 'd')
						i |= MAXIMIZED_B;
					a++;
				}
				client_toggle_state(c, i ? i : (MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B));
				return;
			case A_EXPAND:
				while(a && *a) {
					if(*a == 'h')
						i |= EXPANDED_L | EXPANDED_R;
					if(*a == 'v')
						i |= EXPANDED_T | EXPANDED_B;
					if(*a == 'l')
						i |= EXPANDED_L;
					if(*a == 'r')
						i |= EXPANDED_R;
					if(*a == 'u')
						i |= EXPANDED_T;
					if(*a == 'd')
						i |= EXPANDED_B;
					if(*a == 'a')
						j = true;
					a++;
				}
				client_expand(c, i ? i : (EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B), j);
				return;
			case A_FULLSCREEN:
				client_fullscreen(c);
				return;
			case A_STICKY:
				client_to_desktop(c, (c->desktop == STICKY) ? desktop : STICKY);
				return;
			case A_TITLE:
				client_toggle_title(c);
				return;
			case A_TO_BORDER:
				client_to_border(c, a);
				return;
			case A_ONTOP:
				if(!(c->layer == DESKTOP))
					client_set_layer(c, (c->layer == TOP) ? NORMAL : TOP);
				return;
			case A_BELOW:
				if(!(c->layer == DESKTOP))
					client_set_layer(c, (c->layer == BOTTOM) ? NORMAL : BOTTOM);
				return;
			case A_RAISE:
				client_raise(c);
				return;
			case A_LOWER:
				client_lower(c);
				return;
			case A_CLOSE:
				delete_window(c);
				return;
			case A_MOVE:
			case A_RESIZE:
				if(ev)
					if(ev->type == ButtonPress)
						drag_start(act->code, ev->xbutton.button, ev->xbutton.x_root, ev->xbutton.y_root);
				return;
	}
	switch(act->code) {
	  case A_NEXT:
		case A_PREV:
			wlist_start(ev);
			return;
		case A_ICONIFY_ALL:
			if(cn)
				client_iconify_all();
			return;
		case A_EXEC:
			spawn(a);
			return;
		case A_NEXT_DESKTOP:
			if(desktop < dc - 1)
				desktop_goto(desktop + 1);
			return;
		case A_PREV_DESKTOP:
			if(desktop > 0)
				desktop_goto(desktop - 1);
			return;
		case A_QUIT:
			exit(1);
			return;
	}
}
