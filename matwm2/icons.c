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
      XMoveWindow(dpy, clients[i].parent, ((icon_width + 1) * xo), (display_height + 1) - ((title_height + 5) * (yo + 1)));
      icons_ontop ? XRaiseWindow(dpy, clients[i].parent) : XLowerWindow(dpy, clients[i].parent);
      xo++;
    }
}

void restack_icons(int top) {
  int i;
  for(i = 0; i < cn; i++)
    if(clients[i].iconic)
      top ? XRaiseWindow(dpy, clients[i].parent) : XLowerWindow(dpy, clients[i].parent);
  icons_ontop = top;
}

void iconify(int n) {
  int i;
  XEvent ev;
  if(clients[n].iconic)
    return;
  set_wm_state(clients[n].window, IconicState);
  XUnmapWindow(dpy, clients[n].window);
  clients[n].transient ? XUnmapWindow(dpy, clients[n].parent) : XResizeWindow(dpy, clients[n].parent, icon_width, title_height + 4);
  if(clients[n].shaped)
    XShapeCombineMask(dpy, clients[n].parent, ShapeBounding, 0, 0, None, ShapeSet);
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
  clients[n].transient ? XMapWindow(dpy, clients[n].parent) : XMoveResizeWindow(dpy, clients[n].parent, clients[n].x, clients[n].y, clients[n].width + (border(n) * 2), clients[n].height + (border(n) * 2) + title(n));
  XRaiseWindow(dpy, clients[n].parent);
  XMapWindow(dpy, clients[n].window);
  if(clients[n].shaped)
    XShapeCombineShape(dpy, clients[n].parent, ShapeBounding, border(n), border(n) + title(n), clients[n].window, ShapeBounding, ShapeSet);
  set_wm_state(clients[n].window, NormalState);
  clients[n].iconic = 0;
  sort_icons();
  for(i = 0; i < cn; i++)
    if(clients[i].transient && clients[i].transient_for == clients[n].window)
      restore(i);
}

