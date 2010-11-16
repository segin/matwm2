#include "matwm.h"

client **clients = NULL, **stacking = NULL, *current = NULL;
int cn = 0, nicons = 0;

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
  new->flags = HAS_TITLE | HAS_BORDER | HAS_BUTTONS | CAN_MOVE | CAN_RESIZE;
  new->layer = NORMAL;
  if(have_shape && XShapeQueryExtents(dpy, new->window, &bounding_shaped, &di, &di, &dui, &dui, &di, &di, &di, &dui, &dui) && bounding_shaped)
    new->flags |= SHAPED;
  get_normal_hints(new);
  get_mwm_hints(new);
  get_ewmh_hints(new);
  new->xo = gxo(new, 1);
  new->yo = gyo(new, 1);
  new->x = attr.x - new->xo;
  new->y = attr.y - new->yo;
  if(wm_state == IconicState)
    new->flags |= ICONIC;
  XSelectInput(dpy, w, PropertyChangeMask | EnterWindowMask);
  XShapeSelectInput(dpy, w, ShapeNotifyMask);
  XSetWindowBorderWidth(dpy, w, 0);
  new->parent = XCreateWindow(dpy, root, client_x(new), client_y(new), client_width_total_intern(new), client_height_total_intern(new), (new->flags & HAS_BORDER) ? 1 : 0,
                              DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                              CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWEventMask, &p_attr);
  new->title = XCreateWindow(dpy, new->parent, border_width - 1, border_width - 1, 1, 1, 0,
                             DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                             CWOverrideRedirect | CWEventMask, &p_attr);
  new->wlist_item = XCreateWindow(dpy, wlist, 0, 0, 1, 1, 0,
                                  DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                  CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XFetchName(dpy, w, &new->name);
  client_update_name(new);
  buttons_create(new);
  buttons_update(new);
  client_grab_buttons(new);
  XMapWindow(dpy, new->title);
  if(!(new->flags & DONT_LIST))
    XMapWindow(dpy, new->wlist_item);
  XAddToSaveSet(dpy, w);
  XReparentWindow(dpy, w, new->parent, client_border_intern(new), client_border_intern(new) + client_title(new));
  if(new->flags & FULLSCREEN || new->flags & MAXIMISED)
    client_update_size(new);
  else
    configurenotify(new);
  set_shape(new);
  cn++;
  clients_alloc();
  if(!(new->flags & ICONIC) || new->flags & DONT_LIST) {
    for(i = cn - 1; i > 0 && (stacking[i - 1]->flags & ICONIC || stacking[i - 1]->layer >= new->layer); i--)
      stacking[i] = stacking[i - 1];
    stacking[i] = new;
    clients_apply_stacking();
    if(evh == drag_handle_event)
      client_raise(current);
    XMapRaised(dpy, w);
    XMapWindow(dpy, new->parent);
  } else {
    stacking[cn - 1] = new;
    nicons++;
  }
  clients[cn - 1] = new;
  if(!current)
    client_focus(new);
  if(evh == wlist_handle_event)
    wlist_update();
  ewmh_set_desktop(new, 0);
  ewmh_update_allowed_actions(new);
  ewmh_update_state(new);
}

void client_deparent(client *c) {
  XReparentWindow(dpy, c->window, root, client_x(c) + c->xo, client_y(c) + c->yo);
  XSetWindowBorderWidth(dpy, c->window, c->oldbw);
  XRemoveFromSaveSet(dpy, c->window);
}

void client_remove(client *c) {
  XEvent ev;
  int i;
  if(c->flags & ICONIC)
    nicons--;
  if(button_current == c->button_iconify || button_current == c->button_expand || button_current == c->button_maximise || button_current == c->button_close)
    button_current = None;
  XDestroyWindow(dpy, c->parent);
  XDestroyWindow(dpy, c->wlist_item);
  if(c->name != no_title)
    XFree(c->name);
  for(i = client_number(clients, c) + 1; i < cn; i++)
    clients[i - 1] = clients[i];
  for(i = client_number(stacking, c) + 1; i < cn; i++)
    stacking[i - 1] = stacking[i];
  cn--;
  if(c == current) {
    current = NULL;
    if(cn)
      client_focus(stacking[0]);
  }
  XFreePixmap(dpy, c->title_pixmap);
  free(c);
  clients_alloc();
  if(evh == wlist_handle_event)
    wlist_update();
}

void client_grab_button(client *c, int button) {
  if(!(buttonaction(button) == BA_MOVE && !(c->flags & CAN_MOVE)) && !(buttonaction(button) == BA_RESIZE && !(c->flags & CAN_RESIZE)))
    button_grab(c->parent, button, mousemodmask, ButtonPressMask | ButtonReleaseMask);
}

void client_grab_buttons(client *c) {
  client_grab_button(c, Button1);
  client_grab_button(c, Button2);
  client_grab_button(c, Button3);
  client_grab_button(c, Button4);
  client_grab_button(c, Button5);
}

void client_draw_title(client *c) {
  XFillRectangle(dpy, c->title_pixmap, (c == current) ? bgc : ibgc, 0, 0, c->title_width, text_height);
  XDrawString(dpy, c->title_pixmap, (c == current) ? gc : igc, 0, font->max_bounds.ascent, c->name, strlen(c->name));
}

void client_update_name(client *c) {
  if(!c->name || strlen(c->name) == 0)
    c->name = no_title;
  c->title_width = XTextWidth(font, c->name, strlen(c->name));
  c->title_pixmap = XCreatePixmap(dpy, c->title, c->title_width, text_height, DefaultDepth(dpy, screen));
  client_draw_title(c);
  XSetWindowBackgroundPixmap(dpy, c->title, c->title_pixmap);
  XResizeWindow(dpy, c->title, title_width(c), text_height);
}

void client_set_bg(client *c, XColor color, XColor border) {
  XSetWindowBackground(dpy, c->parent, color.pixel);
  client_draw_title(c);
  XSetWindowBorder(dpy, c->parent, border.pixel);
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
    client_draw_title(c);
  }
  if(evh == wlist_handle_event) {
    XClearWindow(dpy, c->wlist_item);
    wlist_item_draw(c);
  }
}

void clients_apply_stacking(void) {
  int i = 0;
  Window wins[cn + 1];
  wins[0] = wlist;
  for(i = 0; i < cn && !(stacking[i]->flags & ICONIC); i++)
    wins[i + 1] = stacking[i]->parent;
  XRestackWindows(dpy, wins, i + 1);
  ewmh_update_stacking();
}

void client_update_pos(client *c) {
  XMoveWindow(dpy, c->parent, client_x(c), client_y(c));
  configurenotify(c);
}

void client_update_size(client *c) {
  int width = client_width(c);
  buttons_update(c);
  XMoveWindow(dpy, c->button_parent, (width + border_width - 1) - button_parent_width, border_width - 1);
  XResizeWindow(dpy, c->title, title_width(c), text_height);
  XResizeWindow(dpy, c->parent, client_width_total_intern(c), client_height_total_intern(c));
  XResizeWindow(dpy, c->window, width, client_height(c));
  buttons_draw(c);
}

void client_update(client *c) {
  int width = client_width(c);
  XMoveWindow(dpy, c->button_parent, (width + border_width - 1) - button_parent_width, border_width - 1);
  XResizeWindow(dpy, c->title, title_width(c), text_height);
  XMoveResizeWindow(dpy, c->parent, client_x(c), client_y(c), client_width_total_intern(c), client_height_total_intern(c));
  XResizeWindow(dpy, c->window, width, client_height(c));
  buttons_update(c);
  buttons_draw(c);
}

void client_update_title(client *c) {
  XMoveWindow(dpy, c->window, client_border_intern(c), client_border_intern(c) + client_title(c));
  client_update_size(c);
}

void client_warp(client *c) {
  XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, client_width_total_intern(c) - 1, client_height_total_intern(c) - 1);
}

void clients_alloc(void) {
  client **newptr = (client **) realloc((void *) clients, cn * sizeof(client *));
  if(!newptr)
    error();
  clients = newptr;
  newptr = (client **) realloc((void *) stacking, cn * sizeof(client *));
  if(!newptr)
    error();
  stacking = newptr;
}

