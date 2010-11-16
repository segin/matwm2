#include "matwm.h"

Window wlist;
int wlist_width;

void wlist_start(XEvent ev) {
  int i, tl;
  if(evh)
    return;
  wlist_width = 3;
  for(i = 0; i < cn; i++)
    if(clients[i].name) {
      tl = XTextWidth(font, clients[i].name, strlen(clients[i].name)) + 7;
      if(tl > wlist_width)
        wlist_width = tl;
    }
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
  restack_client(current, 1);
  warp();
  evh = NULL;
}

int wlist_handle_event(XEvent ev) {
  int mask;
  switch(ev.type) {
    case KeyPress:
      if(iskey(key_next))
        focus(current + 1 < cn ? current + 1 : 0);
      else if(iskey(key_prev))
        focus(current - 1 >= 0 ? current - 1 : cn - 1);
      else break;
      XWarpPointer(dpy, None, clients[current].icon, 0, 0, 0, 0, wlist_width - 2, 3 + title_height);
      break;
    case KeyRelease:
      mask = key_to_mask(ev.xkey.keycode);
      if(mask) {
        mask = rmbit(ev.xkey.state, mask);
        if(!cmpmask(mask, key_next.mask) && !cmpmask(mask, key_prev.mask))
          wlist_end();
      }
      break;
    default:
      return 0;
  }
  return 1;
}

void wlist_update(void) {
  int i;
  for(i = 0; i < cn; i++)
    XMoveResizeWindow(dpy, clients[i].icon, 1, 1 + ((title_height + 5) * i), wlist_width - 2, title_height + 4);
  XMoveResizeWindow(dpy, wlist, (display_width / 2) - (wlist_width / 2), (display_height / 2) - (1 + ((title_height + 5) * cn) / 2), wlist_width, 1 + ((title_height + 5) * cn));
}

void wlist_draw(void) {
  XDrawRectangle(dpy, wlist, gc, 0, 0, wlist_width - 1, (title_height + 5) * cn);
}

