#include "matwm.h"

void move(int n, int x, int y) {
  XConfigureEvent ce;
  XMoveWindow(dpy, clients[n].parent, x, y);
  clients[n].x = x;
  clients[n].y = y;
  ce.type = ConfigureNotify;
  ce.event = clients[n].window;
  ce.window = clients[n].window;
  ce.x = x;
  ce.y = y;
  ce.width = clients[n].width;
  ce.height = clients[n].height;
  ce.border_width = 0;
  ce.above = None;
  ce.override_redirect = 0;
  XSendEvent(dpy, clients[n].window, False, StructureNotifyMask, (XEvent *) &ce);
}

void resize(int n, int width, int height) {
  if((clients[n].normal_hints.flags & PBaseSize) && (clients[n].normal_hints.flags & PResizeInc)) {
    width -= (width - clients[n].normal_hints.base_width) % clients[n].normal_hints.width_inc;
    height -= (height - clients[n].normal_hints.base_height) % clients[n].normal_hints.height_inc;
  }
  if(clients[n].normal_hints.flags & PMinSize) {
    if(width < clients[n].normal_hints.min_width)
      width = clients[n].normal_hints.min_width;
    if(height < clients[n].normal_hints.min_height)
      height = clients[n].normal_hints.min_height;
  }
  if(clients[n].normal_hints.flags & PMaxSize) {
    if(width > clients[n].normal_hints.max_width)
      width = clients[n].normal_hints.max_width;
    if(height > clients[n].normal_hints.max_height)
      height = clients[n].normal_hints.max_height;
  }
  if(width < MINSIZE)
    width = MINSIZE;
  if(height < MINSIZE)
    height = MINSIZE;
  XResizeWindow(dpy, clients[n].parent, width + (border_width * 2), height + (border_width * 2) + title_height);
  XResizeWindow(dpy, clients[n].window, width, height);
  clients[n].width = width;
  clients[n].height = height;
}

void focus(int n) {
  int i;
  XSetInputFocus(dpy, clients[n].window, RevertToPointerRoot, CurrentTime);
  for(i = 0; i < cn; i++) {
    XSetWindowBackground(dpy, clients[i].parent, i == n ? bg.pixel : ibg.pixel);
    XClearWindow(dpy, clients[i].parent);
    client_draw(i);
  }
}

void delete_window(int n) {
  XEvent ev;
  if(has_protocol(clients[n].window, wm_delete)) {
    ev.type = ClientMessage;
    ev.xclient.window = clients[n].window;
    ev.xclient.message_type = wm_protocols;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = wm_delete;
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, clients[n].window, False, NoEventMask, &ev);
  } else XKillClient(dpy, clients[n].window);
}

