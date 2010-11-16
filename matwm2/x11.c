#include "matwm.h"

int xerrorhandler(Display *display, XErrorEvent *xerror) {
#ifdef DEBUG
  int i;
  char ret[666];
  XGetErrorText(xerror->display, xerror->error_code, ret, 666);
  for(i = 0; i < cn; i++)
    if(clients[i].window == xerror->resourceid)
      printf("client window of %s: ", clients[i].name);
  for(i = 0; i < cn; i++)
    if(clients[i].parent == xerror->resourceid)
      printf("parent window of %s: ", clients[i].name);
  printf("x error: %s\n", ret);
#endif
  if(xerror->error_code == BadAccess && xerror->resourceid == root) {
    fprintf(stderr,"error: root window at display %s is not available\n", XDisplayName(0));
    exit(1);
  }
  return 0;
}

void getnormalhints(int n) {
  long sr;
  XGetWMNormalHints(dpy, clients[n].window, &clients[n].normal_hints, &sr);
}

int getstatehint(Window w) {
  int ret = WithdrawnState;
  XWMHints *wm_hints = XGetWMHints(dpy, w);
  if(wm_hints) {
    if(wm_hints->flags & StateHint)
      ret = wm_hints->initial_state;
    XFree(wm_hints);
  }
  return ret;
}

int get_wm_state(Window w) {
  Atom rt;
  int rf;
  unsigned long n, bar;
  unsigned char *data;
  long ret = WithdrawnState;
  if(XGetWindowProperty(dpy, w, xa_wm_state, 0L, 2L, False, AnyPropertyType, &rt, &rf, &n, &bar, &data) == Success && n) {
    ret = *(long *) data;
    XFree(data);
  }
  return ret;
}

void set_wm_state(Window w, long state) {
  long data[2];
  data[0] = (long) state;
  data[1] = None;
  XChangeProperty(dpy, w, xa_wm_state, xa_wm_state, 32, PropModeReplace, (unsigned char *) data, 2);
}

void configurenotify(int n)
{
  XConfigureEvent ce;
  ce.type = ConfigureNotify;
  ce.event = clients[n].window;
  ce.window = clients[n].window;
  ce.x = clients[n].x + border_width;
  ce.y = clients[n].y + border_width + title_height;
  ce.width = clients[n].width;
  ce.height = clients[n].height;
  ce.border_width = 0;
  ce.above = None;
  ce.override_redirect = 0;
  XSendEvent(dpy, clients[n].window, False, StructureNotifyMask, (XEvent *) &ce);
}

int has_protocol(Window w, Atom protocol) {
  int i, count, ret = 0;
  Atom *protocols;
  if(XGetWMProtocols(dpy, w, &protocols, &count)) {
    for(i = 0; i < count; i++)
      if(protocols[i] == protocol)
        ret++;
    XFree(protocols);
  }
  return ret;
}


void delete_window(int n) {
  XEvent ev;
  if(has_protocol(clients[n].window, xa_wm_delete)) {
    ev.type = ClientMessage;
    ev.xclient.window = clients[n].window;
    ev.xclient.message_type = xa_wm_protocols;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = xa_wm_delete;
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, clients[n].window, False, NoEventMask, &ev);
  } else XKillClient(dpy, clients[n].window);
}

int gxo(int c, int i) {
  if(clients[c].normal_hints.flags & PWinGravity)
    switch(clients[c].normal_hints.win_gravity) {
      case StaticGravity:
        return border_width;
      case NorthGravity:
      case SouthGravity:
      case CenterGravity:
        return border_width + (i ? 0 : (clients[c].width / 2));
      case NorthEastGravity:
      case EastGravity:
      case SouthEastGravity:
        return (border_width * 2) + (i ? 0 : clients[c].width);
    }
  return 0;
}

int gyo(int c, int i) {
  if(clients[c].normal_hints.flags & PWinGravity)
    switch(clients[c].normal_hints.win_gravity) {
      case StaticGravity:
        return border_width + title_height;
      case EastGravity:
      case WestGravity:
      case CenterGravity:
        return border_width + ((title_height + (i ? 0 : clients[c].height)) / 2);
      case SouthEastGravity:
      case SouthGravity:
      case SouthWestGravity:
        return (border_width * 2) + title_height + (i ? 0 : clients[c].height);
    }
  return 0;
}

