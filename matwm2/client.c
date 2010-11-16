#include "matwm.h"

client **clients = NULL, *current = NULL;
int cn = 0;

void add_client(Window w) {
  XWindowAttributes attr;
  int i, di, bounding_shaped, wm_state = get_wm_state(w);
  client *new = (client *) malloc(sizeof(client));
  XGetWindowAttributes(dpy, w, &attr);
  if(wm_state == WithdrawnState) {
    wm_state = getstatehint(w);
    set_wm_state(w, wm_state != WithdrawnState ? wm_state : NormalState);
  }
  new->width = attr.width;
  new->height = attr.height;
  new->oldbw = attr.border_width;
  new->window = w;
  if(have_shape)
    new->shaped = XShapeQueryExtents(dpy, new->window, &bounding_shaped, &di, &di, &di, &di, &di, &di, &di, &di, &di) && bounding_shaped;
  else new->shaped = 0;
  getnormalhints(new);
  get_mwm_hints(new);
  new->x = attr.x - gxo(new, 1);
  new->y = attr.y - gyo(new, 1);
  new->iconic = (wm_state == IconicState) ? 1 : 0;
  new->maximised = 0;
  XFetchName(dpy, w, &new->name);
  if(!new->name)
    new->name = NO_TITLE;
  XSelectInput(dpy, w, PropertyChangeMask | EnterWindowMask);
  XShapeSelectInput(dpy, w, ShapeNotifyMask);
  XSetWindowBorderWidth(dpy, w, 0);
  new->parent = XCreateWindow(dpy, root, new->x, new->y, total_width(new), total_height(new), 0,
                              DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                              CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  new->icon = XCreateWindow(dpy, wlist, 0, 0, 1, 1, 0,
                            DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                            CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, new->parent, border(new), border(new) + title(new));
  grab_button(new->parent, AnyButton, mousemodmask, ButtonPressMask | ButtonReleaseMask);
  configurenotify(new);
  if(new->shaped)
    XShapeCombineShape(dpy, new->parent, ShapeBounding, border(new), border(new) + title(new), new->window, ShapeBounding, ShapeSet);
  cn++;
  alloc_clients();
  if(!new->iconic) {
    XMapWindow(dpy, w);
    XMapRaised(dpy, new->parent);
    for(i = cn - 1; i > 0; i--)
      clients[i] = clients[i - 1];
    clients[0] = new;
  } else clients[cn - 1] = new;
  XMapWindow(dpy, new->icon);
  if(!current)
    focus(new);
  if(evh == wlist_handle_event)
    wlist_update();
}

void remove_client(client *c, int fc) {
  XEvent ev;
  int i;
  if(fc != 2 && XCheckTypedWindowEvent(dpy, c->parent, DestroyNotify, &ev) == False) {
    if(fc)
      set_wm_state(c->window, WithdrawnState);
    XReparentWindow(dpy, c->window, root, c->x + gxo(c, 1), c->y + gyo(c, 1));
    XSetWindowBorderWidth(dpy, c->window, c->oldbw);
    XRemoveFromSaveSet(dpy, c->window);
    XLowerWindow(dpy, c->window);
  }
  XDestroyWindow(dpy, c->parent);
  XDestroyWindow(dpy, c->icon);
  if(c->name != NO_TITLE)
    XFree(c->name);
  for(i = client_number(c); i < cn; i++)
    clients[i] = clients[i + 1];
  cn--;
  if(c == current) {
    current = NULL;
    if(cn)
      focus(clients[0]);
  }
  free(c);
  alloc_clients();
  if(evh == wlist_handle_event)
    wlist_update();
}

void draw_client(client *c) {
  if(c->name && title(c)) {
    XDrawString(dpy, c->parent, (c == current) ? gc : igc, border_width + font->max_bounds.lbearing, border_width + font->max_bounds.ascent, c->name, strlen(c->name));
    XClearArea(dpy, c->parent, c->width + border_width - 1, border_width, border_width, title_height, False);
  }
  if(border(c))
    XDrawRectangle(dpy, c->parent, (c == current) ? gc : igc, 0, 0, c->width + (border_width * 2) - 1, c->height + (border_width * 2) + title(c) - 1);
}

void draw_icon(client *c) {
  if(c->name)
    XDrawString(dpy, c->icon, (c == current) ? gc : igc, 2 + font->max_bounds.lbearing, 2 + font->max_bounds.ascent, c->name, strlen(c->name));
}

void alloc_clients(void) {
  client **newptr = (client **) realloc((void *) clients, cn * sizeof(client *));
  if(!newptr)
    error();
  clients = newptr;
}

void move(client *c, int x, int y) {
  if(x == c->x && y == c->y)
    return;
  c->maximised = 0;
  XMoveWindow(dpy, c->parent, x, y);
  configurenotify(c);
  c->x = x;
  c->y = y;
}

void resize(client *c, int width, int height) {
  if(c->normal_hints.flags & PResizeInc) {
    width -= (width - ((c->normal_hints.flags & PBaseSize) ? c->normal_hints.base_width : 0)) % c->normal_hints.width_inc;
    height -= (height - ((c->normal_hints.flags & PBaseSize) ? c->normal_hints.base_height : 0)) % c->normal_hints.height_inc;
  }
  if(c->normal_hints.flags & PAspect) {
    if(height < (width *  c->normal_hints.min_aspect.y) / c->normal_hints.min_aspect.x)
      height = (width * c->normal_hints.min_aspect.y) / c->normal_hints.min_aspect.x;
    if(height > (width *  c->normal_hints.max_aspect.y) / c->normal_hints.max_aspect.x)
      height = (width * c->normal_hints.max_aspect.y) / c->normal_hints.max_aspect.x;
  }
  if(c->normal_hints.flags & PMinSize) {
    if(width < c->normal_hints.min_width)
      width = c->normal_hints.min_width;
    if(height < c->normal_hints.min_height)
      height = c->normal_hints.min_height;
  }
  if(c->normal_hints.flags & PMaxSize) {
    if(width > c->normal_hints.max_width)
      width = c->normal_hints.max_width;
    if(height > c->normal_hints.max_height)
      height = c->normal_hints.max_height;
  }
  if(width < MINSIZE)
    width = MINSIZE;
  if(height < MINSIZE)
    height = MINSIZE;
  if(width == c->width && height == c->height)
    return;
  c->maximised = 0;
  c->width = width;
  c->height = height;
  XResizeWindow(dpy, c->parent, total_width(c), total_height(c));
  XResizeWindow(dpy, c->window, width, height);
  draw_client(c);
}

void focus(client *c) {
  if(current) {
    XSetWindowBackground(dpy, current->parent, ibg.pixel);
    XSetWindowBackground(dpy, current->icon, ibg.pixel);
    if(!current->iconic)
      XClearWindow(dpy, current->parent);
    XClearWindow(dpy, current->icon);
    draw_client(current);
    draw_icon(current);
  }
  XSetWindowBackground(dpy, c->parent, bg.pixel);
  XSetWindowBackground(dpy, c->icon, bg.pixel);
  if(!c->iconic)
    XClearWindow(dpy, c->parent);
  XClearWindow(dpy, c->icon);
  draw_client(c);
  draw_icon(c);
  if(!c->iconic)
    XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
  current = c;
}

void raise_client(client *c) {
  int i;
  XRaiseWindow(dpy, c->parent);
  for(i = client_number(c); i > 0; i--)
    clients[i] = clients[i - 1];
  clients[0] = c;
}

void lower_client(client *c) {
  int i;
  XLowerWindow(dpy, c->parent);
  for(i = client_number(c); i < cn - 1 && !clients[i + 1]->iconic; i++)
    clients[i] = clients[i + 1];
  clients[i] = c;
}

void maximise(client *c) {
  if(c->maximised) {
    c->maximised = 0;
    move(c, c->prev_x, c->prev_y);
    resize(c, c->prev_width, c->prev_height);
    return;
  }
  c->prev_x = c->x;
  c->prev_y = c->y;
  c->prev_width = c->width;
  c->prev_height = c->height;
  raise_client(current);
  move(c, 0, 0);
  resize(c, display_width - (border(c) * 2), display_height - ((border(c) * 2) + title(c)));
  c->maximised = 1;
}

void set_shape(client *c) {
  int bounding_shaped, di;
  if(c->shaped)
    XShapeCombineShape(dpy, c->parent, ShapeBounding, border(c), border(c) + title(c), c->window, ShapeBounding, ShapeSet);
}

int client_number(client *c) {
  int i;
  for(i = 0; i < cn; i++)
    if(clients[i] == c)
      return i;
}

client *owner(Window w) {
  int i;
  for(i = 0; i < cn; i++)
    if(clients[i]->parent == w || clients[i]->window == w || clients[i]->icon == w)
      return clients[i];
  return NULL;
}

