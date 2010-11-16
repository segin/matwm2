#include "matwm.h"

int desktop = 0;

void desktop_goto(int d) {
  int i;
  for(i = 0; i < cn; i++)
    if(clients[i]->desktop == d) {
      client_show(clients[i]);
      XMapWindow(dpy, clients[i]->wlist_item);
    } else if(clients[i]->desktop == desktop && !(evh == drag_handle_event && clients[i] == current)) {
      client_hide(clients[i]);
      XUnmapWindow(dpy, clients[i]->wlist_item);
    }
  desktop = d;
  if(evh != drag_handle_event) {
    for(i = 0; i < cn; i++)
      if(stacking[i]->desktop == desktop || stacking[i]->desktop == STICKY)
        client_focus(stacking[i]);
  } else {
    current->desktop = desktop;
    ewmh_update_desktop(current);
  }
  ewmh_set_desktop(desktop);
}

void client_to_desktop(client *c, int d) {
  if((c->desktop == desktop || c->desktop == STICKY) && (d != desktop && d != STICKY))
    client_hide(c);
  if((c->desktop != desktop || c->desktop != STICKY) && (d == desktop || d == STICKY))
    client_show(c);
  if(!(c->flags & DONT_LIST)) {
    if((d == desktop || d == ICONS || d == STICKY) && !(c->desktop == desktop || c->desktop == ICONS || c->desktop == STICKY))
      XMapWindow(dpy, c->wlist_item);
    else if(!(d == desktop || d == ICONS || d == STICKY) && (c->desktop == desktop || c->desktop == ICONS || c->desktop == STICKY))
      XUnmapWindow(dpy, c->wlist_item);
  }
  c->desktop = d;
  ewmh_update_desktop(c);
}

