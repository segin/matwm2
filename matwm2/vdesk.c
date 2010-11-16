#include "matwm.h"

int desktop = 0;

void desktop_goto(int d) {
	int i;
	for(i = 0; i < cn; i++)
		if(clients[i]->desktop == d && !(clients[i]->flags & ICONIC)) {
			client_show(clients[i]);
			XMapWindow(dpy, clients[i]->wlist_item);
		} else if(clients[i]->desktop == desktop && !(evh == drag_handle_event && clients[i] == current) && !(clients[i]->flags & ICONIC)) {
			client_hide(clients[i]);
			XUnmapWindow(dpy, clients[i]->wlist_item);
		}
	desktop = d;
	if(evh != drag_handle_event) {
		if(!current)
			for(i = 0; i < cn; i++)
				if(stacking[i]->desktop == desktop || stacking[i]->desktop == STICKY) {
					client_focus(stacking[i]);
					break;
				}
	} else if(current->desktop != STICKY) {
		current->desktop = desktop;
		ewmh_update_desktop(current);
	}
	ewmh_set_desktop(desktop);
}

void client_to_desktop(client *c, int d) {
	int i;
	if((c->desktop == desktop || c->desktop == STICKY) && (d != desktop && d != STICKY) && !(c->flags & ICONIC))	{
		client_hide(c);
		if(!current)
			for(i = 0; i < cn; i++)
				if(stacking[i] != c && (stacking[i]->desktop == desktop || stacking[i]->desktop == STICKY)) {
					client_focus(stacking[i]);
					break;
				}
	}
	if((c->desktop != desktop || c->desktop != STICKY) && (d == desktop || d == STICKY) && !(c->flags & ICONIC))
		client_show(c);
	if(!(c->flags & DONT_LIST) && !(c->flags & ICONIC)) {
		if((d == desktop || d == STICKY) && !(c->desktop == desktop || c->desktop == STICKY))
			XMapWindow(dpy, c->wlist_item);
		else if(!(d == desktop || d == STICKY) && (c->desktop == desktop || c->desktop == STICKY) && c->flags)
			XUnmapWindow(dpy, c->wlist_item);
	}
	c->desktop = d;
	ewmh_update_desktop(c);
}
