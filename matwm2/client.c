#include "matwm.h"

client **clients = NULL, *current = NULL;
int cn = 0;

void client_add(Window w) {
  XWindowAttributes attr;
  int i, di, bounding_shaped, wm_state = get_wm_state(w);
  unsigned int dui;
  client *new = (client *) malloc(sizeof(client));
  wm_state = get_wm_state(w);
  XGetWindowAttributes(dpy, w, &attr);
  if(wm_state == WithdrawnState) {
    wm_state = get_state_hint(w);
    set_wm_state(w, wm_state != WithdrawnState ? wm_state : NormalState);
  }
  new->width = attr.width;
  new->height = attr.height;
  new->oldbw = attr.border_width;
  new->window = w;
  new->flags = HAS_TITLE | HAS_BORDER | HAS_BUTTONS | CAN_RESIZE;
  if(have_shape && XShapeQueryExtents(dpy, new->window, &bounding_shaped, &di, &di, &dui, &dui, &di, &di, &di, &dui, &dui) && bounding_shaped)
    new->flags |= SHAPED;
  get_normal_hints(new);
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
  new->title = XCreateWindow(dpy, new->parent, border_width, border_width, new->width - (button_parent_width + 2), text_height, 0,
                             DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                             CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  new->wlist_item = XCreateWindow(dpy, wlist, 0, 0, 1, 1, 0,
                                  DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                  CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  buttons_create(new);
  buttons_update(new);
  XMapWindow(dpy, new->title);
  XMapWindow(dpy, new->wlist_item);
  XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, new->parent, border(new), border(new) + title(new));
  button_grab(new->parent, AnyButton, mousemodmask, ButtonPressMask | ButtonReleaseMask);
  configurenotify(new);
  set_shape(new);
  cn++;
  clients_alloc();
  if(!(new->flags & ICONIC)) {
    XMapRaised(dpy, w);
    XMapRaised(dpy, new->parent);
    for(i = cn - 1; i > 0; i--)
      clients[i] = clients[i - 1];
    clients[0] = new;
    if(evh != wlist_handle_event && evh != drag_handle_event)
      warpto(new);
  } else clients[cn - 1] = new;
  if(!current)
    client_focus(new);
  if(evh == wlist_handle_event)
    wlist_update();
}

void client_deparent(client *c) {
  XReparentWindow(dpy, c->window, root, c->x + gxo(c, 1), c->y + gyo(c, 1));
  XSetWindowBorderWidth(dpy, c->window, c->oldbw);
  XRemoveFromSaveSet(dpy, c->window);
  XLowerWindow(dpy, c->window);
}

void client_remove(client *c) {
  XEvent ev;
  int i;
  if(button_current == c->button_iconify || button_current == c->button_expand || button_current == c->button_maximise || button_current == c->button_close)
    button_current = root;
  XDestroyWindow(dpy, c->parent);
  XDestroyWindow(dpy, c->wlist_item);
  if(c->name != no_title)
    XFree(c->name);
  for(i = client_number(c) + 1; i < cn; i++)
    clients[i - 1] = clients[i];
  cn--;
  if(c == current) {
    current = NULL;
    if(cn)
      client_focus(clients[0]);
  }
  free(c);
  clients_alloc();
  if(evh == wlist_handle_event)
    wlist_update();
}

void client_draw_border(client *c) {
  if(border(c))
    XDrawRectangle(dpy, c->parent, (c == current) ? gc : igc, 0, 0, c->width + (border_width * 2) - 1, c->height + (border_width * 2) + title(c) - 1);
}

void client_draw_title(client *c) {
  if(c->name && title(c))
    XDrawString(dpy, c->title, (c == current) ? gc : igc, 0, font->max_bounds.ascent, c->name, strlen(c->name));
}

void clients_alloc(void) {
  client **newptr = (client **) realloc((void *) clients, cn * sizeof(client *));
  if(!newptr)
    error();
  clients = newptr;
}

void client_set_bg(client *c, XColor color) {
  XSetWindowBackground(dpy, c->parent, color.pixel);
  XSetWindowBackground(dpy, c->title, color.pixel);
  XSetWindowBackground(dpy, c->button_parent, color.pixel);
  XSetWindowBackground(dpy, c->button_iconify, color.pixel);
  XSetWindowBackground(dpy, c->button_expand, color.pixel);
  XSetWindowBackground(dpy, c->button_maximise, color.pixel);
  XSetWindowBackground(dpy, c->button_close, color.pixel);
  XSetWindowBackground(dpy, c->wlist_item, color.pixel);
  if(!(c->flags & ICONIC)) {
    XClearWindow(dpy, c->parent);
    XClearWindow(dpy, c->title);
    XClearWindow(dpy, c->button_parent);
    buttons_draw(c);
    client_draw_border(c);
    client_draw_title(c);
  }
  if(evh == wlist_handle_event) {
    XClearWindow(dpy, c->wlist_item);
    wlist_item_draw(c);
  }
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
    if(clients[i]->parent == w || clients[i]->window == w || clients[i]->wlist_item == w || clients[i]->title == w || clients[i]->button_parent == w)
      return clients[i];
  return NULL;
}

