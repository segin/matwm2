#include "matwm.h"

client **clients = NULL, *current = NULL;
int cn = 0;

void add_client(Window w) {
  XWindowAttributes attr;
  int i, di, bounding_shaped, wm_state = get_wm_state(w);
  unsigned int dui;
  client *new = (client *) malloc(sizeof(client));
  wm_state = get_wm_state(w);
  XGetWindowAttributes(dpy, w, &attr);
  if(wm_state == WithdrawnState) {
    wm_state = getstatehint(w);
    set_wm_state(w, wm_state != WithdrawnState ? wm_state : NormalState);
  }
  new->width = attr.width;
  new->height = attr.height;
  new->oldbw = attr.border_width;
  new->window = w;
  new->flags = HAS_TITLE | HAS_BORDER | HAS_BUTTONS | CAN_RESIZE;
  if(have_shape && XShapeQueryExtents(dpy, new->window, &bounding_shaped, &di, &di, &dui, &dui, &di, &di, &di, &dui, &dui) && bounding_shaped)
    new->flags |= SHAPED;
  getnormalhints(new);
  get_mwm_hints(new);
  new->x = attr.x - gxo(new, 1);
  new->y = attr.y - gyo(new, 1);
  if(wm_state == IconicState)
    new->flags |= ICONIC;
  XFetchName(dpy, w, &new->name);
  if(!new->name)
    new->name = no_title;
  XSelectInput(dpy, w, PropertyChangeMask | EnterWindowMask);
  XShapeSelectInput(dpy, w, ShapeNotifyMask);
  XSetWindowBorderWidth(dpy, w, 0);
  new->parent = XCreateWindow(dpy, root, new->x, new->y, total_width(new), total_height(new), 0,
                              DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                              CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  new->wlist_item = XCreateWindow(dpy, wlist, 0, 0, 1, 1, 0,
                                  DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                  CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  buttons_create(new);
  if(new->flags & HAS_TITLE) {
    XMapWindow(dpy, new->button_parent);
    new->flags |= HAS_BUTTONS;
  }
  XMapWindow(dpy, new->wlist_item);
  XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, new->parent, border(new), border(new) + title(new));
  grab_button(new->parent, AnyButton, mousemodmask, ButtonPressMask | ButtonReleaseMask);
  configurenotify(new);
  set_shape(new);
  cn++;
  alloc_clients();
  if(!(new->flags & ICONIC)) {
    XMapWindow(dpy, w);
    XMapRaised(dpy, new->parent);
    for(i = cn - 1; i > 0; i--)
      clients[i] = clients[i - 1];
    clients[0] = new;
    warpto(new);
  } else clients[cn - 1] = new;
  if(!current)
    focus(new);
  if(evh == wlist_handle_event)
    wlist_update();
}

void remove_client(client *c) {
  XEvent ev;
  int i;
  XDestroyWindow(dpy, c->parent);
  XDestroyWindow(dpy, c->wlist_item);
  if(c->name != no_title)
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

void deparent_client(client *c) {
  XReparentWindow(dpy, c->window, root, c->x + gxo(c, 1), c->y + gyo(c, 1));
  XSetWindowBorderWidth(dpy, c->window, c->oldbw);
  XRemoveFromSaveSet(dpy, c->window);
  XLowerWindow(dpy, c->window);
}

void draw_client(client *c) {
  if(c->name && title(c)) {
    XDrawString(dpy, c->parent, (c == current) ? gc : igc, border_width, border_width + font->max_bounds.ascent, c->name, strlen(c->name));
    XClearArea(dpy, c->parent, c->width + border_width - 1, border_width, border_width, title_height, False);
    buttons_draw(c);
  }
  if(border(c))
    XDrawRectangle(dpy, c->parent, (c == current) ? gc : igc, 0, 0, c->width + (border_width * 2) - 1, c->height + (border_width * 2) + title(c) - 1);
}

void alloc_clients(void) {
  client **newptr = (client **) realloc((void *) clients, cn * sizeof(client *));
  if(!newptr)
    error();
  clients = newptr;
}

int move(client *c, int x, int y) {
  if(x == c->x && y == c->y)
    return 0;
  XMoveWindow(dpy, c->parent, x, y);
  configurenotify(c);
  c->x = x;
  c->y = y;
  return 1;
}

int resize(client *c, int width, int height) {
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
    return 0;
  c->width = width;
  c->height = height;
  XMoveWindow(dpy, c->button_parent, (c->width + border_width) - button_parent_width, border_width);
  XResizeWindow(dpy, c->parent, total_width(c), total_height(c));
  XResizeWindow(dpy, c->window, width, height);
  draw_client(c);
  return 1;
}

void focus(client *c) {
  client *prev = current;
  current = c;
  if(prev) {
    XSetWindowBackground(dpy, prev->parent, ibg.pixel);
    XSetWindowBackground(dpy, prev->button_parent, ibg.pixel);
    XSetWindowBackground(dpy, prev->button_iconify, ibg.pixel);
    XSetWindowBackground(dpy, prev->button_expand, ibg.pixel);
    XSetWindowBackground(dpy, prev->button_maximise, ibg.pixel);
    XSetWindowBackground(dpy, prev->button_close, ibg.pixel);
    XSetWindowBackground(dpy, prev->wlist_item, ibg.pixel);
    if(!(prev->flags & ICONIC)) {
      XClearWindow(dpy, prev->parent);
      XClearWindow(dpy, prev->button_parent);
      buttons_draw(prev);
      draw_client(prev);
    }
    if(evh == wlist_handle_event) {
      XClearWindow(dpy, prev->wlist_item);
      wlist_item_draw(prev);
    }
  }
  XSetWindowBackground(dpy, c->parent, bg.pixel);
  XSetWindowBackground(dpy, c->button_parent, bg.pixel);
  XSetWindowBackground(dpy, c->button_iconify, bg.pixel);
  XSetWindowBackground(dpy, c->button_expand, bg.pixel);
  XSetWindowBackground(dpy, c->button_maximise, bg.pixel);
  XSetWindowBackground(dpy, c->button_close, bg.pixel);
  XSetWindowBackground(dpy, c->wlist_item, bg.pixel);
  if(!(c->flags & ICONIC)) {
    XClearWindow(dpy, c->parent);
    XClearWindow(dpy, c->button_parent);
    buttons_draw(c);
    draw_client(c);
    XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
  }
  if(evh == wlist_handle_event) {
    XClearWindow(dpy, c->wlist_item);
    wlist_item_draw(c);
  }
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
  for(i = client_number(c); i < cn - 1 && !(clients[i + 1]->flags & ICONIC); i++)
    clients[i] = clients[i + 1];
  clients[i] = c;
}

void maximise(client *c) {
  if(c->flags & MAXIMISED) {
    c->flags ^= MAXIMISED;
    move(c, c->prev_x, c->prev_y);
    resize(c, c->prev_width, c->prev_height);
    return;
  }
  c->prev_x = c->x;
  c->prev_y = c->y;
  c->prev_width = c->width;
  c->prev_height = c->height;
  c->flags |= MAXIMISED;
  raise_client(current);
  move(c, 0, 0);
  resize(c, display_width - (border(c) * 2), display_height - ((border(c) * 2) + title(c)));
}

void expand(client *c) {
  int i, min_x = 0, min_y = 0, max_x = display_width, max_y = display_height;
  if(c->flags & MAXIMISED)
    maximise(c);
  if(c->flags & EXPANDED) {
    c->flags ^= EXPANDED;
    move(c, c->expand_prev_x, c->expand_prev_y);
    resize(c, c->expand_prev_width, c->expand_prev_height);
    return;
  }
  for(i = 0; i < cn; i++) {
    if(clients[i]->flags & ICONIC || c->y >= clients[i]->y + total_height(clients[i]) || c->y + total_height(c) <= clients[i]->y)
      continue;
    if(clients[i]->x + total_width(clients[i]) <= c->x && clients[i]->x + total_width(clients[i]) > min_x)
      min_x = clients[i]->x + total_width(clients[i]);
    if(clients[i]->x >= c->x + total_width(c) && clients[i]->x < max_x)
      max_x = clients[i]->x;
  }
  for(i = 0; i < cn; i++) {
    if(clients[i]->flags & ICONIC || min_x >= clients[i]->x + total_width(clients[i]) || max_x <= clients[i]->x)
      continue;
    if(clients[i]->y + total_height(clients[i]) <= c->y && clients[i]->y + total_height(clients[i]) > min_y)
      min_y = clients[i]->y + total_height(clients[i]);
    if(clients[i]->y >= c->y + total_height(c) && clients[i]->y < max_y)
      max_y = clients[i]->y;
  }
  c->expand_prev_x = c->x;
  c->expand_prev_y = c->y;
  c->expand_prev_width = c->width;
  c->expand_prev_height = c->height;
  move(c, min_x, min_y);
  resize(c, max_x - (min_x + (border(c) * 2)), max_y - (min_y + (border(c) * 2) + title(c)));
  c->flags |= EXPANDED;
}

void toggle_title(client *c) {
  if(c->flags & HAS_TITLE) {
    XUnmapWindow(dpy, c->button_parent);
    c->flags ^= HAS_BUTTONS | HAS_TITLE;
  } else {
    XMapWindow(dpy, c->button_parent);
    c->flags |= HAS_BUTTONS | HAS_TITLE;
  }
  XMoveWindow(dpy, c->window, border(c), border(c) + title(c));
  XResizeWindow(dpy, c->parent, total_width(c), total_height(c));
}

void set_shape(client *c) {
  if(c->flags & SHAPED)
    XShapeCombineShape(dpy, c->parent, ShapeBounding, border(c), border(c) + title(c), c->window, ShapeBounding, ShapeSet);
}

int client_number(client *c) {
  int i;
  for(i = 0; i < cn; i++)
    if(clients[i] == c)
      break;
  return i;
}

client *owner(Window w) {
  int i;
  for(i = 0; i < cn; i++)
    if(clients[i]->parent == w || clients[i]->window == w || clients[i]->wlist_item == w || clients[i]->button_parent == w)
      return clients[i];
  return NULL;
}

