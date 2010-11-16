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

void draw_icon(int n) {
  if(clients[n].name)
    XDrawString(dpy, clients[n].parent, (n == current) ? gc : igc, 2 + font->max_bounds.lbearing, 2 + font->max_bounds.ascent, clients[n].name, strlen(clients[n].name));
  XDrawRectangle(dpy, clients[n].parent, (n == current) ? gc : igc, 0, 0, icon_width - 1, title_height + 3);
  XClearArea(dpy, clients[n].parent, icon_width - 3, 2, 2, title_height, False);
}

void iconify(int n) {
  set_wm_state(clients[n].window, IconicState);
  XUnmapWindow(dpy, clients[n].window);
  XResizeWindow(dpy, clients[n].parent, icon_width, title_height + 4);
  clients[n].iconic = 1;
  sort_icons();
}

void restore(int n) {
  XRaiseWindow(dpy, clients[n].parent);
  XMoveResizeWindow(dpy, clients[n].parent, clients[n].x, clients[n].y, clients[n].width + (border_width * 2), clients[n].height + (border_width * 2) + title_height);
  XMapWindow(dpy, clients[n].window);
  set_wm_state(clients[n].window, NormalState);
  clients[n].iconic = 0;
  sort_icons();
}

