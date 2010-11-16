#include "matwm.h"

client *clients;
int cn = 0, current = 0;

void add_client(Window w) {
  XWindowAttributes attr;
  XGetWindowAttributes(dpy, w, &attr);
  int wm_state = get_wm_state(w);
  if(wm_state == WithdrawnState) {
    wm_state = getstatehint(w);
    set_wm_state(w, wm_state != WithdrawnState ? wm_state : NormalState);
  }
  alloc_clients();
  clients[cn].width = attr.width;
  clients[cn].height = attr.height;
  clients[cn].oldbw = attr.border_width;
  clients[cn].window = w;
  getnormalhints(cn);
  get_mwm_hints(cn);
  clients[cn].x = attr.x - gxo(cn, 1);
  clients[cn].y = attr.y - gyo(cn, 1);
  clients[cn].iconic = (wm_state == IconicState) ? 1 : 0;
  clients[cn].maximised = 0;
  XFetchName(dpy, w, &clients[cn].name);
  XSelectInput(dpy, w, PropertyChangeMask | EnterWindowMask);
  XShapeSelectInput(dpy, w, ShapeNotifyMask);
  XSetWindowBorderWidth(dpy, w, 0);
  clients[cn].parent = XCreateWindow(dpy, root, clients[cn].x, clients[cn].y, (wm_state == IconicState) ? icon_width : (clients[cn].width + (border(cn) * 2)), ((wm_state == IconicState) ? title_height + 4 : (clients[cn].height + (border(cn) * 2) + title(cn))), 0,
                                     DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                     CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, clients[cn].parent, border(cn), border(cn) + title(cn));
  grab_button(clients[cn].parent, AnyButton, mousemodmask, ButtonPressMask | ButtonReleaseMask);
  configurenotify(cn);
  cn++;
  if(wm_state != IconicState) {
    XMapWindow(dpy, w);
    restack_client(cn - 1, 1);
  } else sort_icons();
  XMapWindow(dpy, clients[cn - 1].parent);
  if(current >= cn - 1)
    focus(cn - 1);
  if(have_shape)
    set_shape(cn - 1);
}

void remove_client(int n, int fc) {
  XEvent ev;
  int i, iconic = clients[n].iconic;
  if(fc != 2 && XCheckTypedWindowEvent(dpy, clients[n].parent, DestroyNotify, &ev) == False) {
    if(fc)
      set_wm_state(clients[n].window, WithdrawnState);
    XReparentWindow(dpy, clients[n].window, root, clients[n].x + gxo(n, 1), clients[n].y + gyo(n, 1));
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
  if(clients[n].name && (title(n) || clients[n].iconic))
    XDrawString(dpy, clients[n].parent, (n == current) ? gc : igc, (clients[n].iconic ? 2 : border_width) + font->max_bounds.lbearing, (clients[n].iconic ? 2 : border_width) + font->max_bounds.ascent, clients[n].name, strlen(clients[n].name));
  if(clients[n].border || clients[n].iconic) {
    XDrawRectangle(dpy, clients[n].parent, (n == current) ? gc : igc, 0, 0, (clients[n].iconic ? icon_width : (clients[n].width + (border_width * 2))) - 1, (clients[n].iconic ? title_height + 4 : (clients[n].height + (border_width * 2) + title(n))) - 1);
    XClearArea(dpy, clients[n].parent, clients[n].iconic ? icon_width - 3 : (clients[n].width + border_width - 1), (clients[n].iconic ? 2 : border_width), clients[n].iconic ? 2 : border_width, title_height, False);
  }
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
  clients[n].maximised = 0;
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
  clients[n].maximised = 0;
  if(!clients[n].iconic)
    XResizeWindow(dpy, clients[n].parent, width + (border(n) * 2), height + (border(n) * 2) + title(n));
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
    draw_client(i);
    i = (i != n) ? n : cn;
  }
  if(!clients[n].iconic)
    XSetInputFocus(dpy, clients[n].window, RevertToPointerRoot, CurrentTime);
}

void next(int iconic) {
  int i = current + 1 < cn ? current + 1 : 0;
  while(i < cn && i != current) {
    if(iconic ? clients[i].iconic : !clients[i].iconic) {
      focus(i);
      restack_client(current, 1);
      warp();
      return;
    }
    i++;
    if(current < cn && i == cn)
      i = 0;
  }
}

void prev(int iconic) {
  int i = current - 1 >= 0 ? current - 1 : cn - 1;
  while(i >= 0 && i != current) {
    if(iconic ? clients[i].iconic : !clients[i].iconic) {
      focus(i);
      restack_client(current, 1);
      warp();
      return;
    }
    if(current < cn && !i)
      i = cn;
    i--;
  }
}

void restack_client(int n, int top) {
  int i;
  top ? XRaiseWindow(dpy, clients[n].parent) : XLowerWindow(dpy, clients[n].parent);
  restack_icons(0);
}

void maximise(int n) {
  if(clients[n].maximised) {
    clients[n].maximised = 0;
    move(n, clients[n].prev_x, clients[n].prev_y);
    resize(n, clients[n].prev_width, clients[n].prev_height);
    return;
  }
  clients[n].prev_x = clients[n].x;
  clients[n].prev_y = clients[n].y;
  clients[n].prev_width = clients[n].width;
  clients[n].prev_height = clients[n].height;
  restack_client(current, 1);
  move(n, 0, 0);
  resize(n, display_width - (border(n) * 2), display_height - ((border(n) * 2) + title_height));
  clients[n].maximised = 1;
}

void set_shape(int c) {
  int bounding_shaped, di;
  if(XShapeQueryExtents(dpy, clients[c].window, &bounding_shaped, &di, &di, &di, &di, &di, &di, &di, &di, &di) && bounding_shaped)
    XShapeCombineShape(dpy, clients[c].parent, ShapeBounding, border(c), border(c) + title(c), clients[c].window, ShapeBounding, ShapeSet);
}

