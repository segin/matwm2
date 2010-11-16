#include "matwm.h"

Bool isunmap(Display *display, XEvent *event, XPointer arg) {
  if(event->type == UnmapNotify && event->xunmap.window == *(Window *) arg)
    return True;
  return False;
}

void client_iconify(client *c) {
  int i;
  XEvent ev;
  if(c->flags & ICONIC)
    return;
  set_wm_state(c->window, IconicState);
  XUnmapWindow(dpy, c->parent);
  XUnmapWindow(dpy, c->window);
  XMapWindow(dpy, c->wlist_item);
  c->flags |= ICONIC;
  XIfEvent(dpy, &ev, &isunmap, (XPointer) &c->window);
  for(i = client_number(c); i < cn - 1; i++)
    clients[i] = clients[i + 1];
  clients[cn - 1] = c;
  client_focus(clients[0]);
}

void client_restore(client *c) {
  int i;
  if(!(c->flags & ICONIC))
    return;
  XMapRaised(dpy, c->parent);
  XMapWindow(dpy, c->window);
  set_wm_state(c->window, NormalState);
  c->flags ^= ICONIC;
  for(i = client_number(c); i > 0; i--)
    clients[i] = clients[i - 1];
  clients[0] = c;
  if(c == current)
    XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
}

