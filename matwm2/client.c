#include "matwm.h"
#include <string.h>

client *clients;
int cn = 0, current = 0;

void add_client(Window w, int g) {
  XWindowAttributes attr;
  XGetWindowAttributes(dpy, w, &attr);
  int wm_state = get_wm_state(w);
  if(wm_state == WithdrawnState) {
    wm_state = getstatehint(w);
    set_wm_state(w, wm_state != WithdrawnState ? wm_state : NormalState);
  }
  alloc_clients();
  clients[cn].x = attr.x;
  clients[cn].y = attr.y;
  clients[cn].width = attr.width;
  clients[cn].height = attr.height;
  clients[cn].oldbw = attr.border_width;
  clients[cn].window = w;
  clients[cn].minimised = (wm_state == IconicState) ? 1 : 0;
  XFetchName(dpy, w, &clients[cn].name);
  getnormalhints(cn);
  XSelectInput(dpy, w, PropertyChangeMask);
  XSetWindowBorderWidth(dpy, w, 0);
  clients[cn].parent = XCreateWindow(dpy, root, clients[cn].x - (g ? gxo(cn, 1) : 0), clients[cn].y - (g ? gyo(cn, 1) : 0), clients[cn].width + (border_width * 2), clients[cn].height + (border_width * 2) + title_height, 0,
                                     DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                     CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  clients[cn].taskbutton = XCreateWindow(dpy, taskbar, 0, 0, taskbutton_width, title_height + 4, 0,
                                         DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                         CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, clients[cn].parent, border_width, border_width + title_height);
  XMapWindow(dpy, w);
  grab_button(clients[cn].parent, AnyButton, mousemodmask, ButtonPressMask | ButtonReleaseMask);
  configurenotify(cn);
  cn++;
  if(wm_state == IconicState) {
    XMapWindow(dpy, clients[cn - 1].taskbutton);
    update_taskbar();
  } else XMapRaised(dpy, clients[cn - 1].parent);
  if(current >= cn - 1)
    focus(cn - 1);
}

void update_taskbar(void) {
  int i, xo = 0, yo = 0;
  for(i = 0; i < cn; i++) 
    if(clients[i].minimised) {
      if(xo == taskbar_width) {
        xo = 0;
        yo++;
      }
      XMoveWindow(dpy, clients[i].taskbutton, 1 + ((taskbutton_width + 1) * xo), 1 + ((title_height + 5) * yo));
      xo++;
    }
  XMoveResizeWindow(dpy, taskbar, 0, XDisplayHeight(dpy, screen) - (1 + ((title_height + 5) * (yo + 1))), 1 + (((taskbutton_width + 1) * (yo ? taskbar_width : xo))),  1 + ((title_height + 5) * (yo + 1)));
  if(!taskbar_visible && xo) {
    XMapWindow(dpy, taskbar);
    taskbar_visible = 1;
  } else if(taskbar_visible && !xo) {
    XUnmapWindow(dpy, taskbar);
    taskbar_visible = 0;
  }
}

void remove_client(int n) {
  unsigned int i, nwins;
  Window dw1, dw2, *wins;
  XQueryTree(dpy, clients[n].parent, &dw1, &dw2, &wins, &nwins);
  if(wins != NULL) {
    for(i = 0; i < nwins; i++)
      if(wins[i] == clients[n].window) {
        XReparentWindow(dpy, clients[n].window, root, clients[n].x, clients[n].y);
        XSetWindowBorderWidth(dpy, clients[n].window, clients[n].oldbw);
        XRemoveFromSaveSet(dpy, clients[n].window);
      }
    XFree(wins);
  }
  XDestroyWindow(dpy, clients[n].taskbutton);
  XDestroyWindow(dpy, clients[n].parent);
  XFree(clients[n].name);
  cn--;
  for(i = n; i < cn; i++)
    clients[i] = clients[i + 1];
  alloc_clients();
  update_taskbar();
}

void draw_client(int n) {
  if(clients[n].name)
    XDrawString(dpy, clients[n].parent, (n == current) ? gc : igc, border_width + font->max_bounds.lbearing, border_width + font->max_bounds.ascent, clients[n].name, strlen(clients[n].name));
  XDrawRectangle(dpy, clients[n].parent, (n == current) ? gc : igc, 0, 0, clients[n].width + (border_width * 2) - 1, clients[n].height + (border_width * 2) + title_height - 1);
  XClearArea(dpy, clients[n].parent, clients[n].width + border_width - 1, border_width, border_width, title_height, False);
}

void draw_taskbutton(int n) {
  if(clients[n].name)
    XDrawString(dpy, clients[n].taskbutton, (n == current) ? gc : igc, 2 + font->max_bounds.lbearing, 2 + font->max_bounds.ascent, clients[n].name, strlen(clients[n].name));
  XClearArea(dpy, clients[n].taskbutton, taskbutton_width - 2, 2, 2, title_height, False);
}

void add_initial_clients(void) {
  unsigned int i, nwins;
  Window dw1, dw2, *wins;
  XWindowAttributes attr;
  XQueryTree(dpy, root, &dw1, &dw2, &wins, &nwins);
  for(i = 0; i < nwins; i++) {
    XGetWindowAttributes(dpy, wins[i], &attr);
    if(!attr.override_redirect && attr.map_state == IsViewable)
      add_client(wins[i], 0);
  }
  if(wins != NULL)
    XFree(wins);
}

void alloc_clients(void) {
  clients = (client *) realloc((client *) clients, (cn + 1) * sizeof(client));
  if(!clients) {
    fprintf(stderr, "error: allocating memory failed\n");
    end();
    exit(1);
  }
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

void getnormalhints(int n) {
  long sr;
  XGetWMNormalHints(dpy, clients[n].window, &clients[n].normal_hints, &sr);
}

int getstatehint(Window w) {
  int ret = WithdrawnState;
  XWMHints *wm_hints = XGetWMHints(dpy, w);
  if(wm_hints) {
    ret = wm_hints->initial_state;
    XFree(wm_hints);
  }
  return ret;
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

void configure(int c, XConfigureRequestEvent *e) {
  if(c < cn) {
    resize(c, (e->value_mask & CWWidth) ? e->width : clients[c].width, (e->value_mask & CWHeight) ? e->height : clients[c].height);
    move(c, (e->value_mask & CWX) ? e->x - gxo(c, 0) : clients[c].x, (e->value_mask & CWY) ? e->y - gyo(c, 0) : clients[c].y);
  } else {
    XWindowChanges wc;
    wc.sibling = e->above;
    wc.stack_mode = e->detail;
    wc.x = e->x;
    wc.y = e->y;
    wc.width = e->width;
    wc.height = e->height;
    XConfigureWindow(dpy, e->window, e->value_mask, &wc);
  }
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

void move(int n, int x, int y) {
  if(x == clients[n].x && y == clients[n].y)
    return;
  XMoveWindow(dpy, clients[n].parent, x, y);
  configurenotify(n);
  clients[n].x = x;
  clients[n].y = y;
}

void resize(int n, int width, int height) {
  if(clients[n].normal_hints.flags & PResizeInc) {
    width -= (width - ((clients[n].normal_hints.flags & PBaseSize) ? clients[n].normal_hints.base_width : 0)) % clients[n].normal_hints.width_inc;
    height -= (height - ((clients[n].normal_hints.flags & PBaseSize) ? clients[n].normal_hints.base_height : 0)) % clients[n].normal_hints.height_inc;
  }
  if(clients[n].normal_hints.flags & PAspect) {
    if(height < (width *  clients[n].normal_hints.min_aspect.y) / clients[n].normal_hints.min_aspect.x)
      height = (width * clients[n].normal_hints.min_aspect.y) / clients[n].normal_hints.min_aspect.x;
    if(height > (width *  clients[n].normal_hints.max_aspect.y) / clients[n].normal_hints.max_aspect.x)
      height = (width * clients[n].normal_hints.max_aspect.y) / clients[n].normal_hints.max_aspect.x;
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
  if(width == clients[n].width && height == clients[n].height)
    return;
  XResizeWindow(dpy, clients[n].parent, width + (border_width * 2), height + (border_width * 2) + title_height);
  XResizeWindow(dpy, clients[n].window, width, height);
  clients[n].width = width;
  clients[n].height = height;
  draw_client(n);
}

void focus(int n) {
  int i = current < cn ? current : n;
  current = n;
  while(i < cn) {
    XSetWindowBackground(dpy, clients[i].parent, i == n ? bg.pixel : ibg.pixel);
    XSetWindowBackground(dpy, clients[i].taskbutton, i == n ? bg.pixel : ibg.pixel);
    XClearWindow(dpy, clients[i].minimised ? clients[i].taskbutton : clients[i].parent);
    clients[i].minimised ? draw_taskbutton(i) : draw_client(i);
    i = (i != n) ? n : cn;
  }
  if(!clients[n].minimised)
    XSetInputFocus(dpy, clients[n].window, RevertToPointerRoot, CurrentTime);
}

void next(int minimised, int warp) {
  int i = current + 1 < cn ? current + 1 : 0;
  while(i < cn && i != current) {
    if(minimised ? clients[i].minimised : !clients[i].minimised) {
      focus(i);
      XRaiseWindow(dpy, clients[current].parent);
      if(warp)
        XWarpPointer(dpy, None, minimised ? clients[current].taskbutton : clients[current].parent, 0, 0, 0, 0, minimised ? taskbutton_width - 1 : clients[current].width + border_width,  (minimised ? 3 : (clients[current].height + border_width)) + title_height);
      return;
    }
    i++;
    if(current < cn && i == cn)
      i = 0;
  }
}

void prev(int minimised, int warp) {
  int i = current - 1 < cn ? current - 1 : cn -1;
  while(i >= 0 && i != current) {
    if(minimised ? clients[i].minimised : !clients[i].minimised) {
      focus(i);
      XRaiseWindow(dpy, clients[current].parent);
      if(warp)
        XWarpPointer(dpy, None, minimised ? clients[current].taskbutton : clients[current].parent, 0, 0, 0, 0, minimised ? taskbutton_width - 1 : clients[current].width + border_width,  (minimised ? 3 : (clients[current].height + border_width)) + title_height);
      return;
    }
    if(current < cn && !i)
      i = cn;
    i--;
  }
}

void minimise(int n) {
  if(clients[n].minimised) {
    XMapRaised(dpy, clients[n].parent);
    XUnmapWindow(dpy, clients[n].taskbutton);
    set_wm_state(clients[n].window, NormalState);
    clients[n].minimised = 0;
    update_taskbar();
    return;
  }
  set_wm_state(clients[n].window, IconicState);
  XUnmapWindow(dpy, clients[n].parent);
  clients[n].minimised = 1;
  update_taskbar();
  XMapWindow(dpy, clients[n].taskbutton);
}

