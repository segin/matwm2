#include "matwm.h"

Bool isunmap(Display *display, XEvent *event, XPointer arg) {
  if(event->type == UnmapNotify && event->xunmap.window == *(Window *) arg)
    return True;
  return False;
}

void iconify(client *c) {
  int i;
  XEvent ev;
  if(c->state & ICONIC)
    return;
  set_wm_state(c->window, IconicState);
  XUnmapWindow(dpy, c->parent);
  XUnmapWindow(dpy, c->window);
  XMapWindow(dpy, c->wlist_item);
  c->state |= ICONIC;
  XIfEvent(dpy, &ev, &isunmap, (XPointer) &c->window);
  for(i = client_number(c); i < cn - 1; i++)
    clients[i] = clients[i + 1];
  clients[cn - 1] = c;
  focus(clients[0]);
}

void restore(client *c) {
  int i;
  if(!(c->state & ICONIC))
    return;
  XMapRaised(dpy, c->parent);
  XMapWindow(dpy, c->window);
  set_wm_state(c->window, NormalState);
  c->state ^= ICONIC;
  for(i = client_number(c); i > 0; i--)
    clients[i] = clients[i - 1];
  clients[0] = c;
}

