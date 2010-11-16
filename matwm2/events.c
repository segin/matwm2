#include "matwm.h"

int (*evh)(XEvent) = NULL;
Time lastclick = 0;
unsigned int lastbutton = None;
client *lastclick_client = NULL;

void handle_event(XEvent ev) {
	client *c = owner(ev.xany.window);
	int i = 0, j = 0;
	char *a;
#ifdef DEBUG_EVENTS
	if(c) printf("%i (%s): %s\n", ev.xany.window, c->name, event_name(ev));
	else printf("%i: %s\n", ev.xany.window, event_name(ev));
#endif
	if((evh && evh(ev)) || button_handle_event(ev) || ewmh_handle_event(ev))
		return;
	if(c) {
		if(!has_child(c->parent, c->window) && ev.type != DestroyNotify && ev.type != UnmapNotify)
			return;
		switch(ev.type) {
			case UnmapNotify:
				if(c->window == ev.xunmap.window) {
					if(has_child(c->parent, c->window)) {
						client_deparent(c);
						set_wm_state(c->window, WithdrawnState);
					}
					client_remove(c);
					ewmh_update_clist();
				}
				return;
			case PropertyNotify:
				if(ev.xproperty.atom == XA_WM_NAME) {
					if(c->name != no_title)
						XFree(c->name);
					XFreePixmap(dpy, c->title_pixmap);
					XFetchName(dpy, c->window, &c->name);
					client_update_name(c);
					XClearWindow(dpy, c->title);
					if(evh == wlist_handle_event) {
						XClearWindow(dpy, c->wlist_item);
						wlist_item_draw(c);
					}
				}
				if(ev.xproperty.atom == XA_WM_NORMAL_HINTS)
					get_normal_hints(c);
				return;
			case ClientMessage:
				if(ev.xclient.message_type == xa_wm_change_state && ev.xclient.data.l[0] == IconicState)
					client_iconify(c);
				return;
			case EnterNotify:
				if(c != current && !(c->flags & CLICK_FOCUS) && !click_focus && ev.xcrossing.detail != NotifyInferior && (ev.xcrossing.window == c->parent || ev.xcrossing.window == c->wlist_item) && ev.xcrossing.send_event == False)
					client_focus(c);
				return;
			case Expose:
				if(ev.xexpose.count == 0 && evh == wlist_handle_event && c && ev.xexpose.window == c->wlist_item)
					wlist_item_draw(c);
				return;
			case ButtonPress:
				XAllowEvents(dpy, ReplayPointer, CurrentTime);
				if(ev.xbutton.window != c->window) {
					if(lastclick + doubleclick_time > ev.xbutton.time && lastbutton == ev.xbutton.button && lastclick_client == c)
						client_handle_button(c, ev, 1);
					lastclick = ev.xbutton.time;
					lastclick_client = c;
					lastbutton = ev.xbutton.button;
				}
				if(c != current)
					client_focus(c);
				if(ev.xbutton.window == c->window) {
					if(click_raise)
						client_raise(c);
					return;
				}
				client_handle_button(c, ev, 0);
				return;
			case FocusOut:
				if(c == current && ev.xfocus.mode != NotifyGrab)
					XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
				return;
#ifdef SHAPE
			default:
				if(ev.type == shape_event) {
					set_shape(c);
					return;
				}
#endif
		}
	}
	if(current && ev.type == KeyPress)
		switch(keyaction(ev)) {
			case KA_ICONIFY:
				client_iconify(current);
				return;
			case KA_MAXIMIZE:
				a = keyarg(ev);
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
				client_toggle_state(current, i ? i : (MAXIMIZED_L | MAXIMIZED_R | MAXIMIZED_T | MAXIMIZED_B));
				return;
			case KA_EXPAND:
				a = keyarg(ev);
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
						j = 1;
					a++;
				}
				client_expand(current, i ? i : (EXPANDED_L | EXPANDED_R | EXPANDED_T | EXPANDED_B), j);
				return;
			case KA_FULLSCREEN:
				client_fullscreen(current);
				return;
			case KA_STICKY:
				client_to_desktop(current, (current->desktop == STICKY) ? desktop : STICKY);
				return;
			case KA_TITLE:
				client_toggle_title(current);
				return;
			case KA_TO_BORDER:
				client_to_border(current, keyarg(ev));
				return;
			case KA_ONTOP:
				if(!(current->layer == DESKTOP))
					client_set_layer(current, (current->layer == TOP) ? NORMAL : TOP);
				return;
			case KA_BELOW:
				if(!(current->layer == DESKTOP))
					client_set_layer(current, (current->layer == BOTTOM) ? NORMAL : BOTTOM);
				return;
			case KA_RAISE:
				client_raise(current);
				return;
			case KA_LOWER:
				client_lower(current);
				return;
			case KA_CLOSE:
				delete_window(current);
				return;
		}
	switch(ev.type) {
		case MapRequest:
			c = owner(ev.xmaprequest.window);
			if(c) {
				if(c->flags & ICONIC && has_child(c->parent, c->window)) {
					client_restore(c);
					if(focus_new)
						client_focus(c);
				}
			} else if(has_child(root, ev.xmaprequest.window))
				client_add(ev.xmaprequest.window);
			return;
		case DestroyNotify:
			c = owner(ev.xdestroywindow.window);
			if(c && c->window == ev.xdestroywindow.window)
				client_remove(c);
			return;
		case ConfigureRequest:
			c = owner(ev.xconfigurerequest.window);
			if(c) {
				if(!has_child(c->parent, c->window))
					return;
				if(ev.xconfigurerequest.value_mask & CWX)
					c->x = ev.xconfigurerequest.x - gxo(c, 0);
				if(ev.xconfigurerequest.value_mask & CWY)
					c->y = ev.xconfigurerequest.y - gyo(c, 0);
				if(ev.xconfigurerequest.value_mask & CWWidth)
					c->width = ev.xconfigurerequest.width;
				if(ev.xconfigurerequest.value_mask & CWHeight)
					c->height = ev.xconfigurerequest.height;
				client_update(c);
			} else if(has_child(root, ev.xconfigurerequest.window)) {
				XWindowChanges wc;
				wc.sibling = ev.xconfigurerequest.above;
				wc.stack_mode = ev.xconfigurerequest.detail;
				wc.x = ev.xconfigurerequest.x;
				wc.y = ev.xconfigurerequest.y;
				wc.width = ev.xconfigurerequest.width;
				wc.height = ev.xconfigurerequest.height;
				XConfigureWindow(dpy, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &wc);
			}
			return;
		case MappingNotify:
			if(ev.xmapping.request != MappingPointer) {
				keys_ungrab();
				XRefreshKeyboardMapping(&ev.xmapping);
				keys_update();
			}
			return;
		case KeyPress:
			if(keyaction(ev) == KA_NEXT || keyaction(ev) == KA_PREV)
				wlist_start(ev);
			if(cn && keyaction(ev) == KA_ICONIFY_ALL)
				client_iconify_all();
			if(keyaction(ev) == KA_EXEC)
				spawn(keyarg(ev));
			if(keyaction(ev) == KA_NEXT_DESKTOP && desktop < dc - 1)
				desktop_goto(desktop + 1);
			if(keyaction(ev) == KA_PREV_DESKTOP && desktop > 0)
				desktop_goto(desktop - 1);
			return;
		case ConfigureNotify:
			if(root == ev.xconfigure.window) {
				display_width = ev.xconfigure.width;
				display_height = ev.xconfigure.height;
				if(evh == wlist_handle_event)
					wlist_update();
				ewmh_update_geometry();
				for(i = 0; i < cn; i++)
					client_update_size(clients[i]);
			}
			return;
	}
}

