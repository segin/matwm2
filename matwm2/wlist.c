#include "matwm.h"

Window wlist;
int wlist_width, wlist_screen;
client *client_before_wlist;

void wlist_start(XEvent *ev) {
	#ifdef DEBUG
	printf(NAME ": wlist_start(): entering window list mode\n");
	#endif
	if(evh || !cn)
		return;
	if(!wlist_update())
		return;
	screens_update_current();
	wlist_screen = cs;
	XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
	XSetInputFocus(dpy, None, RevertToPointerRoot, CurrentTime);
	evh = wlist_handle_event;
	client_before_wlist = current;
	wlist_update();
	XMapWindow(dpy, wlist);
	wlist_handle_event(ev);
}

void wlist_end(int err) {
	#ifdef DEBUG
	printf(NAME ": wlist_end(): ending window list mode\n");
	#endif
	XUngrabKeyboard(dpy, CurrentTime);
	evh = NULL;
	if(current && !err) {
		if(client_before_wlist && client_before_wlist != current)
			previous = client_before_wlist;
		client_save(current);
		if(current->flags & ICONIC)
			client_restore(current);
		else
			client_raise(current);
		client_warp(current);
		take_focus(current);
	}
	XUnmapWindow(dpy, wlist);
}

client *wlist_next(void) {
	int i = current ? client_number(stacking, current) + 1 : 0;
	for(i = (i < cn) ? i : 0; stacking[i]->flags & DONT_LIST || !(stacking[i]->desktop == desktop || stacking[i]->flags & ICONIC || stacking[i]->desktop == STICKY); i++)
		if(i >= cn - 1)
			i = -1;
	return stacking[i];
}

client *wlist_prev(void) {
	int i = current ? client_number(stacking, current) - 1 : cn - 1;
	for(i = (i >= 0) ? i : cn - 1; stacking[i]->flags & DONT_LIST || !(stacking[i]->desktop == desktop || stacking[i]->flags & ICONIC || stacking[i]->desktop == STICKY); i--)
		if(i <= 0)
			i = cn - 1;
	return stacking[i];
}

bool wlist_handle_event(XEvent *ev) {
	int mask, i;
	client *c;
	action *a;
	switch(ev->type) {
		case KeyPress:
			#ifdef DEBUG_EVENTS
			printf(NAME ": wlist_handle_event(): handling KeyPress event\n");
			#endif
			a = keyaction(ev);
			if(!a)
				break;
			if(a->code == A_NEXT) {
				client_focus(wlist_next(), false);
			} else if(a->code == A_PREV) {
				client_focus(wlist_prev(), false);
			} else break;
			XWarpPointer(dpy, None, current->wlist_item, 0, 0, 0, 0, wlist_width - 2, wlist_item_height - 1);
			break;
		case KeyRelease:
			#ifdef DEBUG_EVENTS
			printf(NAME ": wlist_handle_event(): handling KeyRelease event\n");
			#endif
			mask = ev->xkey.state;
			if(mask) {
				mask ^= ev->xkey.state & key_to_mask(ev->xkey.keycode);
				for(i = 0; i < keyn; i++)
					if((keys[i].a->code == A_NEXT || keys[i].a->code == A_PREV) && (keys[i].mask & mask) == keys[i].mask)
						break;
				if(i == keyn)
					wlist_end(0);
			}
			break;
		case ButtonPress:
			#ifdef DEBUG_EVENTS
			printf(NAME ": wlist_handle_event(): handling ButtonPress event\n");
			#endif
			if(click_focus) {
				c = owner(ev->xbutton.window);
				if(c)
					client_focus(c, false);
			}
			break;
		case ButtonRelease:
			break;
		default:
			return false;
	}
	return true;
}

int wlist_update(void) {
	int i, wlist_height = 1, nl = 0, item_width;
	wlist_width = 3;
	for(i = 0; i < cn; i++)
		if(!(stacking[i]->flags & DONT_LIST) && (client_visible(stacking[i]) || stacking[i]->flags & ICONIC)) {
			item_width = stacking[i]->title_width + 2 + (wlist_margin * 2);
			if(item_width > wlist_width)
				wlist_width = stacking[i]->title_width + 2 + (wlist_margin * 2);
		} else nl++;
	if(nl == cn) { /* no clients that have to be listed */
		wlist_end(1);
		return 0;
	}
	if(wlist_width > screens[wlist_screen].width)
		wlist_width = screens[wlist_screen].width;
	if(wlist_maxwidth && wlist_width > wlist_maxwidth + 2) /* add two because this specifies maximum width of items, not the list */
		wlist_width = wlist_maxwidth + 2;
	for(i = 0; i < cn; i++) {
		if(i == cn - nicons)
			wlist_height++;
		if(!(stacking[i]->flags & DONT_LIST) && (client_visible(stacking[i]) || stacking[i]->flags & ICONIC)) {
			XMoveResizeWindow(dpy, stacking[i]->wlist_item, 1, wlist_height, wlist_width - 2, wlist_item_height);
			wlist_height += wlist_item_height + 1;
		}
	}
	XMoveResizeWindow(dpy, wlist, screens[wlist_screen].x + ((screens[wlist_screen].width / 2) - (wlist_width / 2)), screens[wlist_screen].y + ((screens[wlist_screen].height / 2) - (wlist_height / 2)), wlist_width, wlist_height);
	return 1;
}

void wlist_item_draw(client *c) {
	int space = wlist_width - (2 + (wlist_margin * 2));
	int center = (c->title_width < space) ? (space / 2) - (c->title_width / 2) : 0;
	#ifdef USE_XFT
	if(xftfont) {
		XClearWindow(dpy, c->wlist_item);
	  XftDrawString8(c->wlist_draw, (c == current) ? &xftfg : &xftifg, xftfont, wlist_margin + (center_wlist_items ? center : 0), wlist_margin + xftfont->ascent, (unsigned char *) c->name, strlen(c->name));
	} else
	#endif
	XDrawString(dpy, c->wlist_item, (c == current) ? gc : igc, wlist_margin + (center_wlist_items ? center : 0), wlist_margin + font->max_bounds.ascent, c->name, strlen(c->name));
}
