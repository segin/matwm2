#include "matwm.h"

int icons_ontop = 0;

void sort_icons(void) {
  int i, xo = 0, yo = 0;
  for(i = 0; i < cn; i++) 
    if(clients[i].iconic) {
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

void draw_icon(int n) {
  if(clients[n].name)
    XDrawString(dpy, clients[n].icon, (n == current) ? gc : igc, 2 + font->max_bounds.lbearing, 2 + font->max_bounds.ascent, clients[n].name, strlen(clients[n].name));
  XDrawRectangle(dpy, clients[n].icon, (n == current) ? gc : igc, 0, 0, icon_width - 1, title_height + 3);
  XClearArea(dpy, clients[n].icon, icon_width - 3, 2, 2, title_height, False);
}

void minimise(int n) {
  if(clients[n].iconic) {
    XMapRaised(dpy, clients[n].parent);
    XUnmapWindow(dpy, clients[n].icon);
    set_wm_state(clients[n].window, NormalState);
    clients[n].iconic = 0;
    sort_icons();
    return;
  }
  set_wm_state(clients[n].window, IconicState);
  XUnmapWindow(dpy, clients[n].parent);
  clients[n].iconic = 1;
  sort_icons();
  XMapWindow(dpy, clients[n].icon);
}

