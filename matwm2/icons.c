#include "matwm.h"

int icons_ontop = 0;

void sort_icons(void) {
  int i, xo = 0, yo = 0;
  for(i = 0; i < cn; i++) 
    if(clients[i].iconic && !clients[i].transient) {
      if(xo == hmaxicons) {
        xo = 0;
        yo++;
      }
      XMoveWindow(dpy, clients[i].icon, ((icon_width + 1) * xo), (display_height + 1) - ((title_height + 5) * (yo + 1)));
      icons_ontop ? XRaiseWindow(dpy, clients[i].icon) : XLowerWindow(dpy, clients[i].icon);
      xo++;
    }
}

void restack_icons(int top) {
  int i;
  for(i = 0; i < cn; i++)
    if(clients[i].iconic)
      top ? XRaiseWindow(dpy, clients[i].icon) : XLowerWindow(dpy, clients[i].icon);
  icons_ontop = top;
}

void iconify(int n) {
  int i;
  XEvent ev;
  if(clients[n].iconic)
    return;
  set_wm_state(clients[n].window, IconicState);
  XUnmapWindow(dpy, clients[n].window);
  XUnmapWindow(dpy, clients[n].parent);
  if(!clients[n].transient)
    XMapWindow(dpy, clients[n].icon);
  clients[n].iconic = 1;
  sort_icons();
  while(!XCheckTypedWindowEvent(dpy, clients[n].parent, UnmapNotify, &ev));
  for(i = 0; i < cn; i++)
    if(clients[i].transient && clients[i].transient_for == clients[n].window && !clients[i].iconic)
      iconify(i);
}

void restore(int n) {
  int i;
  if(!clients[n].iconic)
    return;
  if(!clients[n].transient)
    XUnmapWindow(dpy, clients[n].icon);
  XMapRaised(dpy, clients[n].parent);
  XMapWindow(dpy, clients[n].window);
  set_wm_state(clients[n].window, NormalState);
  clients[n].iconic = 0;
  sort_icons();
  for(i = 0; i < cn; i++)
    if(clients[i].transient && clients[i].transient_for == clients[n].window)
      restore(i);
}

