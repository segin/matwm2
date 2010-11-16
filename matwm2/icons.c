#include "matwm.h"

int icons_ontop = 0;

void sort_icons(void) {
  int i, xo = 0, yo = 0;
  for(i = 0; i < cn; i++) 
    if(clients[i].iconic == 1) {
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
  sort_icons();
  XIfEvent(dpy, &ev, &isunmap, (XPointer) &clients[n].window);
}

void restore(int n) {
  int i;
  if(!clients[n].iconic)
    return;
  if(clients[n].iconic == 1)
    XUnmapWindow(dpy, clients[n].icon);
  XMapRaised(dpy, clients[n].parent);
  XMapWindow(dpy, clients[n].window);
  set_wm_state(clients[n].window, NormalState);
  clients[n].iconic = 0;
  sort_icons();
}

