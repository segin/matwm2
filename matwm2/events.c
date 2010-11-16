#include "matwm.h"

bool (*evh)(XEvent *) = NULL;
Time lastclick = 0;
unsigned int lastbutton = None;
client *lastclick_client = NULL;

void handle_event(XEvent *ev) {
	client *c = owner(ev->xany.window);
	#ifdef DEBUG_EVENTS
	if(ev->type != Expose && ev->type != MotionNotify && !(ev->type == ConfigureNotify && ev->xconfigure.window != root)) /* this makes the output slightly more manageable */
		printf(NAME ": handle_event(): got %s\n\twindow: 0x%X (%s)\n", event_name(ev), (unsigned int) ev->xany.window, c ? c->name : ((ev->xany.window == root) ? "root" : "unknown"));
	#endif
	if((evh && evh(ev)) || button_handle_event(ev) || ewmh_handle_event(ev) || screens_handle_event(ev)) {
		#ifdef DEBUG_EVENTS
		if(ev->type != Expose && ev->type != MotionNotify && (ev->type != ConfigureNotify && ev->xconfigure.window != root))
			printf(NAME ": handle_event(): external event handler claimed last event\n");
		#endif
		return;
	}
	if(c) {
		if(!has_child(c->parent, c->window) && ev->type != DestroyNotify && ev->type != UnmapNotify)
			return;
		switch(ev->type) {
			case UnmapNotify:
				if(c->window == ev->xunmap.window) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handle_event(): handling UnmapNotify event\n\twindow: 0x%X (%s)\n", (unsigned int) c->window, c->name);
					#endif
					if(has_child(c->parent, c->window)) {
						client_deparent(c);
						set_wm_state(c->window, WithdrawnState);
					}
					client_remove(c);
					ewmh_update_clist();
				}
				return;
			case PropertyNotify:
				if(ev->xproperty.atom == XA_WM_NAME) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handle_event(): handling PropertyNotify event\n\twindow: 0x%X (%s)\n\tproperty: XA_WM_NAME\n", (unsigned int) c->window, c->name);
					#endif
					if(c->name != no_title)
						XFree(c->name);
					#ifdef USE_XFT
					if(xftfont)
						XftDrawDestroy(c->title_draw);
					#endif
					XFreePixmap(dpy, c->title_pixmap);
					XFetchName(dpy, c->window, &c->name);
					client_update_name(c);
					XClearWindow(dpy, c->title);
					if(evh == wlist_handle_event) {
						XClearWindow(dpy, c->wlist_item);
						wlist_item_draw(c);
					}
				}
				if(ev->xproperty.atom == XA_WM_NORMAL_HINTS) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handle_event(): handling PropertyNotify event\n\twindow: 0x%X (%s)\n\tproperty: XA_WM_NORMAL_HINTS\n", (unsigned int) c->window, c->name);
					#endif
					get_normal_hints(c);
				}
				return;
			case ClientMessage:
				if(ev->xclient.message_type == xa_wm_change_state && ev->xclient.data.l[0] == IconicState) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handling ClientMessage event\n\twindow: 0x%X (%s)\n", (unsigned int) c->window, c->name);
					#endif
					client_iconify(c);
				}
				return;
			case EnterNotify:
				if(c != current && !(c->flags & CLICK_FOCUS) && !click_focus && ev->xcrossing.mode != NotifyGrab && ev->xcrossing.mode != NotifyUngrab && ev->xcrossing.detail != NotifyInferior && (ev->xcrossing.window == c->parent || ev->xcrossing.window == c->wlist_item) && ev->xcrossing.send_event == False) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handle_event(): handling EnterNotify event\n\twindow: 0x%X (%s)\n", (unsigned int) c->window, c->name);
					#endif
					client_focus(c, true);
				}
				return;
			case Expose:
				if(ev->xexpose.count == 0 && evh == wlist_handle_event && c && ev->xexpose.window == c->wlist_item)
					wlist_item_draw(c);
				return;
			case ButtonPress:
				#ifdef DEBUG_EVENTS
				printf(NAME ": handle_event(): handling ButtonPress event\n\twindow: 0x%X (%s)\n", (unsigned int) c->window, c->name);
				#endif
				if(c != current)
					client_focus(c, true);
				XAllowEvents(dpy, ReplayPointer, CurrentTime);
				if(ev->xbutton.window != c->window) {
					if(lastclick + doubleclick_time > ev->xbutton.time && lastbutton == ev->xbutton.button && lastclick_client == c) {
						client_action(c, buttonaction(ev->xbutton.button, true), ev);
						lastclick = 0;
						lastclick_client = NULL;
						lastbutton = None;
						return;
					}
					lastclick = ev->xbutton.time;
					lastclick_client = c;
					lastbutton = ev->xbutton.button;
					client_action(c, buttonaction(ev->xbutton.button, false), ev);
				} else if(click_raise)
					client_raise(c);
				return;
			case FocusIn: /* we ignore pointer events, these happen if the input focus is on the root window */
				if(allow_focus_stealing && c != current && ev->xfocus.mode != NotifyGrab && ev->xfocus.mode != NotifyUngrab && ev->xfocus.detail != NotifyPointer) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handle_event(): handling FocusIn event\n\twindow: 0x%X (%s)\n", (unsigned int) c->window, c->name);
					#endif
					client_focus(c, false);
				}
				return;
			case FocusOut:
				if(c == current && ev->xfocus.mode != NotifyGrab && ev->xfocus.mode != NotifyUngrab && ev->xfocus.detail != NotifyInferior) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handle_event(): handling FocusOut event\n\twindow: 0x%X (%s)\n", (unsigned int) c->window, c->name);
					#endif
					if(allow_focus_stealing && ev->xfocus.detail != NotifyAncestor) {
						#ifdef DEBUG_EVENTS
						printf("\tfocus lost\n");
						#endif
						client_focus(NULL, false); /* we do this so windows that aren't managed can take focus */
					} else {
						#ifdef DEBUG_EVENTS
						printf("\tre-focussing this window\n");
						#endif
						take_focus(c);
					}
				}
				return;
			#ifdef USE_SHAPE
			default:
				if(ev->type == shape_event) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handle_event(): handling ShapeNotify event\n\twindow 0x%X (%s)\n", (unsigned int) c->window, c->name);
					#endif
					set_shape(c);
					return;
				}
			#endif
		}
	}
	switch(ev->type) {
		case MapRequest:
			c = owner(ev->xmaprequest.window);
			#ifdef DEBUG_EVENTS
			printf(NAME ": handle_event(): handling MapRequest event\n\twindow: 0x%X (%s)\n", (unsigned int) ev->xmaprequest.window, c ? c->name : "unknown");
			#endif
			if(c) {
				if(c->flags & ICONIC && has_child(c->parent, c->window)) {
					client_restore(c);
					if(focus_new)
						client_focus(c, true);
				}
			} else if(has_child(root, ev->xmaprequest.window))
				client_add(ev->xmaprequest.window, false);
			return;
		case DestroyNotify:
			c = owner(ev->xdestroywindow.window);
			if(c)
				if(c->window == ev->xdestroywindow.window) {
					#ifdef DEBUG_EVENTS
					printf(NAME ": handle_event(): handling DestroyNotify event\n\twindow 0x%X (%s)\n", (unsigned int) c->window, c->name);
					#endif
					client_remove(c);
				}
			return;
		case ConfigureRequest:
			c = owner(ev->xconfigurerequest.window);
			#ifdef DEBUG_EVENTS
			printf(NAME ": handle_event(): handling ConfigureRequest event\n\twindow 0x%X (%s)\n", (unsigned int) ev->xconfigurerequest.window, c ? c->name : "unknown");
			#endif
			if(correct_center)
				screens_correct_center(&ev->xconfigurerequest.x, &ev->xconfigurerequest.y, &ev->xconfigurerequest.width, &ev->xconfigurerequest.height);
			if(c) {
				if(!has_child(c->parent, c->window))
					return;
				if(ev->xconfigurerequest.value_mask & CWX)
					c->x = ev->xconfigurerequest.x - gxo(c, false);
				if(ev->xconfigurerequest.value_mask & CWY)
					c->y = ev->xconfigurerequest.y - gyo(c, false);
				if(ev->xconfigurerequest.value_mask & CWWidth)
					c->width = ev->xconfigurerequest.width;
				if(ev->xconfigurerequest.value_mask & CWHeight)
					c->height = ev->xconfigurerequest.height;
				client_update(c);
				#ifdef DEBUG
				printf(NAME ": handle_event(): reconfigured client 0x%X (%s) to %ix%i+%i+%i\n", (unsigned int) c->window, c->name, c->width, c->height, c->x, c->y);
				#endif
			} else if(has_child(root, ev->xconfigurerequest.window)) {
				XWindowChanges wc;
				wc.sibling = ev->xconfigurerequest.above;
				wc.stack_mode = ev->xconfigurerequest.detail;
				wc.x = ev->xconfigurerequest.x;
				wc.y = ev->xconfigurerequest.y;
				wc.width = ev->xconfigurerequest.width;
				wc.height = ev->xconfigurerequest.height;
				XConfigureWindow(dpy, ev->xconfigurerequest.window, ev->xconfigurerequest.value_mask, &wc);
			}
			return;
		case MapNotify:
			if(correct_center_unmanaged && ev->xany.window == root && !owner(ev->xmap.window)) {
				#ifdef DEBUG_EVENTS
				printf(NAME ": handle_event(): handling MapNotify event\n\twindow 0x%X (unknown)\n", (unsigned int) ev->xmap.window);
				#endif
				window_correct_center(ev->xmap.window);
			}
			break;
		case MappingNotify:
			if(ev->xmapping.request != MappingPointer) {
				#ifdef DEBUG_EVENTS
				printf(NAME ": handle_event(): handling MappingNotify event\n");
				#endif
				keys_ungrab();
				XRefreshKeyboardMapping(&ev->xmapping);
				keys_update();
			}
			return;
		case KeyPress:
			#ifdef DEBUG_EVENTS
			printf(NAME ": handle_event(): handling KeyPress event\n");
			#endif
			client_action(current, keyaction(ev), ev);
			return;
		case ButtonPress:
			if(ev->xbutton.window != root)
				return;
			#ifdef DEBUG_EVENTS
			printf(NAME ": handle_event(): handling ButtonPress event\n");
			#endif
			if(lastclick + doubleclick_time > ev->xbutton.time && lastbutton == ev->xbutton.button && lastclick_client == NULL) {
				client_action(current, root_buttonaction(ev->xbutton.button, true), ev);
				lastclick = 0;
				lastclick_client = NULL;
				lastbutton = None;
				return;
			}
			lastclick = ev->xbutton.time;
			lastclick_client = NULL;
			lastbutton = ev->xbutton.button;
			client_action(current, root_buttonaction(ev->xbutton.button, false), ev);
			return;
	}
}
