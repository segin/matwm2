#include "matwm.h"

Bool isunmap(Display *display, XEvent *event, XPointer arg) {
  if(event->type == UnmapNotify && event->xunmap.window == *(Window *) arg)
    return True;
  return False;
}

void iconify(int n) {
  int i;
  XEvent ev;
  if(clients[n].iconic)
    return;
  set_wm_state(clients[n].window, IconicState);
  XUnmapWindow(dpy, clients[n].window);
  XUnmapWindow(dpy, clients[n].parent);
  XMapWindow(dpy, clients[n].icon);
  clients[n].iconic = 1;
  XIfEvent(dpy, &ev, &isunmap, (XPointer) &clients[n].window);
}

void restore(int n) {
  int i;
  if(!clients[n].iconic)
    return;
  XMapRaised(dpy, clients[n].parent);
  XMapWindow(dpy, clients[n].window);
  set_wm_state(clients[n].window, NormalState);
  clients[n].iconic = 0;
}

