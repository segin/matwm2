#include "matwm.h"

Window wlist;
int wlist_width;
client *client_before_wlist;

void wlist_start(XEvent ev) {
	if(evh || !cn)
		return;
	if(!wlist_update())
		return;
	XMapRaised(dpy, wlist);
	XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
	evh = wlist_handle_event;
	client_before_wlist = current;
	handle_event(ev);
}

void wlist_end(int err) {
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
		XSetInputFocus(dpy, current->window, RevertToPointerRoot, CurrentTime);
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

int wlist_handle_event(XEvent ev) {
	int mask, i;
	client *c;
	switch(ev.type) {
		case KeyPress:
			i = keyaction(ev);
			if(i == KA_NEXT) {
				client_focus(wlist_next());
			} else if(i == KA_PREV) {
				client_focus(wlist_prev());
			} else break;
			XWarpPointer(dpy, None, current->wlist_item, 0, 0, 0, 0, wlist_width - 2, 3 + title_height);
			break;
		case KeyRelease:
			mask = ev.xkey.state;
			if(mask) {
				mask ^= ev.xkey.state & key_to_mask(ev.xkey.keycode);
				for(i = 0; i < keyn; i++)
					if((keys[i].action == KA_NEXT || keys[i].action == KA_PREV) && cmpmodmask(keys[i].mask, mask))
						break;
				if(i == keyn)
					wlist_end(0);
			}
			break;
		case ButtonPress:
			if(click_focus) {
				c = owner(ev.xbutton.window);
				if(c)
					client_focus(c);
			}
			break;
		case ButtonRelease:
			break;
		default:
			return 0;
	}
	return 1;
}

int wlist_update(void) {
	int i, nc = 0, offset = 1, nl = 0;
	wlist_width = 3;
	for(i = 0; i < cn; i++)
		if(!(stacking[i]->flags & DONT_LIST) && (client_visible(stacking[i]) || stacking[i]->flags & ICONIC)) {
			if(stacking[i]->title_width + 6 > wlist_width)
				wlist_width = stacking[i]->title_width + 6;
		} else nl++;
	if(nl == cn) { /* no clients that have to be listed */
		wlist_end(1);
		return 0;
	}
	if(wlist_width > display_width)
		wlist_width = display_width;
	for(i = 0; i < cn; i++) {
		if(i == cn - nicons)
			offset = 2;
		if(!(stacking[i]->flags & DONT_LIST) && (client_visible(stacking[i]) || stacking[i]->flags & ICONIC)) {
			XMoveResizeWindow(dpy, stacking[i]->wlist_item, 1, offset + ((title_height + 5) * nc), wlist_width - 2, title_height + 4);
			nc++;
		}
	}
	XMoveResizeWindow(dpy, wlist, (display_width / 2) - (wlist_width / 2), (display_height / 2) - (1 + ((title_height + 5) * nc) / 2), wlist_width, offset + ((title_height + 5) * nc));
	return 1;
}

void wlist_item_draw(client *c) {
	if(c->name)
		XDrawString(dpy, c->wlist_item, (c == current) ? gc : igc, center_wlist_items ? (wlist_width / 2) - c->title_width / 2 : 2, 2 + font->max_bounds.ascent, c->name, strlen(c->name));
}

