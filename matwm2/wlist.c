#include "matwm.h"

Window wlist;
int wlist_width;

void wlist_start(XEvent ev) {
  if(evh || !cn)
    return;
  wlist_width = 3;
  wlist_update();
  XMapRaised(dpy, wlist);
  XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
  evh = wlist_handle_event;
  handle_event(ev);
}

void wlist_end(void) {
  XUngrabKeyboard(dpy, CurrentTime);
  XUnmapWindow(dpy, wlist);
  restore(current);
  raise_client(current);
  warp();
  evh = NULL;
}

int wlist_handle_event(XEvent ev) {
  int mask, i;
  switch(ev.type) {
    case KeyPress:
      if(iskey(key_next)) {
        focus((client_number(current) < cn - 1) ? clients[client_number(current) + 1] : clients[0]);
      } else if(iskey(key_prev)) {
        focus((client_number(current) > 0) ? clients[client_number(current) - 1] : clients[cn - 1]);
      } else break;
      XWarpPointer(dpy, None, current->icon, 0, 0, 0, 0, wlist_width - 2, 3 + title_height);
      break;
    case KeyRelease:
      mask = key_to_mask(ev.xkey.keycode);
      if(mask) {
        mask = rmbit(ev.xkey.state, mask);
        if(!cmpmask(mask, key_next.mask) && !cmpmask(mask, key_prev.mask))
          wlist_end();
      }
    case ButtonPress:
    case ButtonRelease:
      break;
    default:
      return 0;
  }
  return 1;
}

void wlist_update(void) {
  int i, tl, nc = 0, offset = 1;
  if(!cn)
    wlist_end();
  for(i = 0; i < cn; i++)
    if(clients[i]->name) {
      tl = XTextWidth(font, clients[i]->name, strlen(clients[i]->name)) + 7;
      if(tl > wlist_width)
        wlist_width = tl;
    }
  for(i = 0; i < cn; i++) {
    if(clients[i]->iconic)
      offset = 2;
    XMoveResizeWindow(dpy, clients[i]->icon, 1, offset + ((title_height + 5) * nc), wlist_width - 2, title_height + 4);
    nc++;
  }
  XMoveResizeWindow(dpy, wlist, (display_width / 2) - (wlist_width / 2), (display_height / 2) - (1 + ((title_height + 5) * cn) / 2), wlist_width, offset + ((title_height + 5) * cn));
}

