#include "matwm.h"

client *clients;
int cn = 0, current = 0;

void add_client(Window w, int new) {
  XWindowAttributes attr;
  XGetWindowAttributes(dpy, w, &attr);
  int wm_state = get_wm_state(w);
  if(new && wm_state == WithdrawnState) {
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
  clients[cn].iconic = (wm_state == IconicState) ? 1 : 0;
  XFetchName(dpy, w, &clients[cn].name);
  getnormalhints(cn);
  XSelectInput(dpy, w, PropertyChangeMask);
  XSetWindowBorderWidth(dpy, w, 0);
  clients[cn].parent = XCreateWindow(dpy, root, clients[cn].x - (new ? gxo(cn, 1) : 0), clients[cn].y - (new ? gyo(cn, 1) : 0), (wm_state == IconicState) ? icon_width : (clients[cn].width + (border_width * 2)), title_height + ((wm_state == IconicState) ? 4 : (clients[cn].height + (border_width * 2))), 0,
                                     DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                     CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, clients[cn].parent, border_width, border_width + title_height);
  grab_button(clients[cn].parent, AnyButton, mousemodmask, ButtonPressMask | ButtonReleaseMask);
  configurenotify(cn);
  cn++;
  if(wm_state != IconicState) {
    XMapWindow(dpy, w);
    XRaiseWindow(dpy, w);
  } sort_icons();
  XMapWindow(dpy, clients[cn - 1].parent);
  if(current >= cn - 1)
    focus(cn - 1);
}

void remove_client(int n, int fc) {
  XEvent ev;
  int i, iconic = clients[n].iconic;
  if(XCheckTypedWindowEvent(dpy, clients[n].parent, DestroyNotify, &ev) == False) {
    if(fc)
      set_wm_state(clients[n].window, WithdrawnState);
    XReparentWindow(dpy, clients[n].window, root, clients[n].x, clients[n].y);
    XSetWindowBorderWidth(dpy, clients[n].window, clients[n].oldbw);
    XRemoveFromSaveSet(dpy, clients[n].window);
  }
  XDestroyWindow(dpy, clients[n].parent);
  XFree(clients[n].name);
  cn--;
  for(i = n; i < cn; i++)
    clients[i] = clients[i + 1];
  alloc_clients();
  if(iconic)
    sort_icons();
}

void draw_client(int n) {
  if(clients[n].name)
    XDrawString(dpy, clients[n].parent, (n == current) ? gc : igc, border_width + font->max_bounds.lbearing, border_width + font->max_bounds.ascent, clients[n].name, strlen(clients[n].name));
  XDrawRectangle(dpy, clients[n].parent, (n == current) ? gc : igc, 0, 0, clients[n].width + (border_width * 2) - 1, clients[n].height + (border_width * 2) + title_height - 1);
  XClearArea(dpy, clients[n].parent, clients[n].width + border_width - 1, border_width, border_width, title_height, False);
}

void alloc_clients(void) {
  clients = (client *) realloc((client *) clients, (cn + 1) * sizeof(client));
  if(!clients) {
    fprintf(stderr, "error: allocating memory failed\n");
    end();
    exit(1);
  }
}

void move(int n, int x, int y) {
  if(x == clients[n].x && y == clients[n].y)
    return;
  if(!clients[n].iconic)
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
  if(!clients[n].iconic)
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
    XClearWindow(dpy, clients[i].parent);
    clients[i].iconic ? draw_icon(i) : draw_client(i);
    i = (i != n) ? n : cn;
  }
  if(!clients[n].iconic)
    XSetInputFocus(dpy, clients[n].window, RevertToPointerRoot, CurrentTime);
}

void next(int iconic, int warp) {
  int i = current + 1 < cn ? current + 1 : 0;
  while(i < cn && i != current) {
    if(iconic ? clients[i].iconic : !clients[i].iconic) {
      focus(i);
      restack_client(current, 1);
      if(warp)
        XWarpPointer(dpy, None, clients[current].parent, 0, 0, 0, 0, iconic ? icon_width - 1 : clients[current].width + border_width,  (iconic ? 3 : (clients[current].height + border_width)) + title_height);
      return;
    }
    i++;
    if(current < cn && i == cn)
      i = 0;
  }
}

void prev(int iconic, int warp) {
  int i = current - 1 < cn ? current - 1 : cn -1;
  while(i >= 0 && i != current) {
    if(iconic ? clients[i].iconic : !clients[i].iconic) {
      focus(i);
      restack_client(current, 1);
      if(warp)
        XWarpPointer(dpy, None, clients[current].parent, 0, 0, 0, 0, iconic ? icon_width - 1 : clients[current].width + border_width,  (iconic ? 3 : (clients[current].height + border_width)) + title_height);
      return;
    }
    if(current < cn && !i)
      i = cn;
    i--;
  }
}

void restack_client(int c, int top) {
  top ? XRaiseWindow(dpy, clients[c].parent) : XLowerWindow(dpy, clients[c].parent);
  restack_icons(0);
}

