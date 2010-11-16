#include "matwm.h"

int client_move(client *c, int x, int y) {
  if(x == c->x && y == c->y)
    return 0;
  XMoveWindow(dpy, c->parent, x, y);
  c->x = x;
  c->y = y;
  configurenotify(c);
  return 1;
}

int client_resize(client *c, int width, int height) {
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
  buttons_update(c);
  XMoveWindow(dpy, c->button_parent, (c->width + border_width) - button_parent_width, border_width);
  XResizeWindow(dpy, c->title, c->width - ((c->flags & HAS_BUTTONS) ? button_parent_width + 2 : 0), text_height);
  XResizeWindow(dpy, c->parent, total_width(c), total_height(c));
  XResizeWindow(dpy, c->window, width, height);
  client_draw_border(c);
  client_draw_title(c);
  return 1;
}

void client_focus(client *c) {
  client *prev = current;
  current = c;
  if(prev)
    client_set_bg(prev, ibg);
  client_set_bg(c, bg);
  if(!(c->flags & ICONIC) && isviewable(c->window))
    XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
}

void client_raise(client *c) {
  int i;
  XRaiseWindow(dpy, c->parent);
  for(i = client_number(c); i > 0; i--)
    clients[i] = clients[i - 1];
  clients[0] = c;
}

void client_lower(client *c) {
  int i;
  XLowerWindow(dpy, c->parent);
  for(i = client_number(c); i < cn - 1 && !(clients[i + 1]->flags & ICONIC); i++)
    clients[i] = clients[i + 1];
  clients[i] = c;
}

void client_maximise(client *c) {
  if(c->flags & MAXIMISED) {
    c->flags ^= MAXIMISED;
    client_move(c, c->prev_x, c->prev_y);
    client_resize(c, c->prev_width, c->prev_height);
    return;
  }
  c->prev_x = c->x;
  c->prev_y = c->y;
  c->prev_width = c->width;
  c->prev_height = c->height;
  c->flags |= MAXIMISED;
  client_raise(current);
  client_move(c, 0, 0);
  client_resize(c, display_width - (border(c) * 2), display_height - ((border(c) * 2) + title(c)));
}

void client_expand(client *c) {
  int i, min_x = 0, min_y = 0, max_x = display_width, max_y = display_height;
  if(c->flags & MAXIMISED)
    client_maximise(c);
  if(c->flags & EXPANDED) {
    c->flags ^= EXPANDED;
    client_move(c, c->expand_prev_x, c->expand_prev_y);
    client_resize(c, c->expand_prev_width, c->expand_prev_height);
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
  client_move(c, min_x, min_y);
  client_resize(c, max_x - (min_x + (border(c) * 2)), max_y - (min_y + (border(c) * 2) + title(c)));
  c->flags |= EXPANDED;
}

void client_toggle_title(client *c) {
  c->flags ^= HAS_TITLE;
  XMoveWindow(dpy, c->window, border(c), border(c) + title(c));
  XResizeWindow(dpy, c->parent, total_width(c), total_height(c));
}

void client_iconify(client *c) {
  int i;
  XEvent ev;
  if(c->flags & ICONIC)
    return;
  set_wm_state(c->window, IconicState);
  XUnmapWindow(dpy, c->parent);
  XUnmapWindow(dpy, c->window);
  XMapWindow(dpy, c->wlist_item);
  c->flags |= ICONIC;
  XIfEvent(dpy, &ev, &isunmap, (XPointer) &c->window);
  if(current == c && evh == drag_handle_event)
    evh = drag_release_wait;
  for(i = client_number(c); i < cn - 1; i++)
    clients[i] = clients[i + 1];
  clients[cn - 1] = c;
  client_focus(clients[0]);
}

void client_restore(client *c) {
  int i;
  if(!(c->flags & ICONIC))
    return;
  XMapRaised(dpy, c->parent);
  XMapWindow(dpy, c->window);
  set_wm_state(c->window, NormalState);
  c->flags ^= ICONIC;
  for(i = client_number(c); i > 0; i--)
    clients[i] = clients[i - 1];
  clients[0] = c;
  if(c == current)
    XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
}

void client_save(client *c) {
  int x = c->x, y = c->y;
  if(c->x >= display_width)
    x = display_width - total_width(c);
  if(c->y >= display_height)
    y = display_height - total_height(c);
  if(c->x + total_width(c) <= 0)
    x = 0;
  if(c->y + total_height(c) <= 0)
    y = 0;
  client_move(c, x, y);
}

