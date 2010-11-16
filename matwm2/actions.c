#include "matwm.h"

int client_move(client *c, int x, int y) {
  if(x == c->x && y == c->y)
    return 0;
  c->width = client_width(c);
  c->height = client_height(c);
  if(current->flags & (MAXIMISED | FULLSCREEN | EXPANDED)) {
    c->flags ^= current->flags & (MAXIMISED | FULLSCREEN | EXPANDED);
    ewmh_update_state(c);
  }
  c->x = x;
  c->y = y;
  client_update_pos(c);
  return 1;
}

int client_resize(client *c, int width, int height) {
  c->x = client_x(c);
  c->y = client_y(c);
  if(current->flags & (MAXIMISED | FULLSCREEN | EXPANDED)) {
    c->flags ^= c->flags & (MAXIMISED | FULLSCREEN | EXPANDED);
    ewmh_update_state(c);
  }
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
  client_update_size(c);
  return 1;
}

void client_focus(client *c) {
  client *prev = current;
  current = c;
  if(prev)
    client_set_bg(prev, ibg, ifg);
  client_set_bg(c, bg, fg);
  if(c->desktop == desktop || c->desktop == STICKY && isviewable(c->window))
    XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
  ewmh_set_active(c);
}

void client_raise(client *c) {
  int i;
  for(i = client_number(stacking, c); i > 0 && (stacking[i - 1]->layer >= c->layer || stacking[i - 1]->desktop == ICONS); i--)
    stacking[i] = stacking[i - 1];
  stacking[i] = c;
  clients_apply_stacking();
}

void client_lower(client *c) {
  int i;
  for(i = client_number(stacking, c); i < cn - 1 && stacking[i + 1]->layer <= c->layer && stacking[i + 1]->desktop != ICONS; i++)
    stacking[i] = stacking[i + 1];
  stacking[i] = c;
  clients_apply_stacking();
}

void client_set_layer(client *c, int layer) {
  int i, prev = c->layer;
  c->layer = layer;
  if(layer > prev)
    for(i = client_number(stacking, c); i < cn - 1 && stacking[i + 1]->layer < c->layer && stacking[i + 1]->desktop != ICONS; i++)
      stacking[i] = stacking[i + 1];
  else
    for(i = client_number(stacking, c); i > 0 && stacking[i - 1]->layer > c->layer; i--)
      stacking[i] = stacking[i - 1];
  stacking[i] = c;
  clients_apply_stacking();
  ewmh_update_state(c);
}

void client_maximise(client *c) {
  if(!(c->flags & CAN_MOVE) || !(c->flags & CAN_RESIZE))
    return;
  if(c->flags & MAXIMISED)
    c->flags ^= MAXIMISED;
  else {
    c->flags |= MAXIMISED;
    client_raise(c);
  }
  client_update(c);
  ewmh_update_state(c);
}

void client_fullscreen(client *c) {
  if(!(c->flags & CAN_MOVE) || !(c->flags & CAN_RESIZE))
    return;
  if(c->flags & FULLSCREEN)
    c->flags ^= FULLSCREEN;
  else {
    c->flags |= FULLSCREEN;
    client_raise(c);
  }
  client_update(c);
  ewmh_update_state(c);
}

void client_expand(client *c) {
  int i;
  if(!(c->flags & CAN_MOVE) || !(c->flags & CAN_RESIZE))
    return;
  if(c->flags & EXPANDED) {
    c->flags ^= EXPANDED;
    client_update(c);
    return;
  }
  c->expand_x = 0;
  c->expand_y = 0;
  c->expand_width = display_width;
  c->expand_height = display_height;
  for(i = 0; i < cn; i++) {
    if((clients[i]->desktop != STICKY && clients[i]->desktop != desktop) || c->y >= client_y(clients[i]) + client_height_total(clients[i]) || c->y + (c->height + (client_border(c) * 2) + client_title(c)) <= client_y(clients[i]))
      continue;
    if(client_x(clients[i]) + client_width_total(clients[i]) <= c->x && client_x(clients[i]) + client_width_total(clients[i]) > c->expand_x)
      c->expand_x = client_x(clients[i]) + client_width_total(clients[i]);
    if(client_x(clients[i]) >= c->x + (c->width + (client_border(c) * 2)) && client_x(clients[i]) < c->expand_width)
      c->expand_width = client_x(clients[i]);
  }
  for(i = 0; i < cn; i++) {
    if((clients[i]->desktop != STICKY && clients[i]->desktop != desktop) || c->expand_x >= client_x(clients[i]) + client_width_total(clients[i]) || c->expand_width <= client_x(clients[i]))
      continue;
    if(client_y(clients[i]) + client_height_total(clients[i]) <= c->y && client_y(clients[i]) + client_height_total(clients[i]) > c->expand_y)
      c->expand_y = client_y(clients[i]) + client_height_total(clients[i]);
    if(client_y(clients[i]) >= c->y + (c->height + (client_border(c) * 2) + client_title(c)) && client_y(clients[i]) < c->expand_height)
      c->expand_height = client_y(clients[i]);
  }
  c->expand_width -= c->expand_x + (client_border(c) * 2);
  c->expand_height -= c->expand_y + (client_border(c) * 2) + client_title(c);
  c->flags |= EXPANDED;
  client_update(c);
}

void client_toggle_title(client *c) {
  c->flags ^= HAS_TITLE;
  client_update_title(c);
}

void client_iconify(client *c) {
  int i;
  if(c->desktop == ICONS || c->flags & DONT_LIST)
    return;
  nicons++;
  set_wm_state(c->window, IconicState);
  client_to_desktop(c, ICONS);
  for(i = client_number(stacking, c); i < cn - 1; i++)
    stacking[i] = stacking[i + 1];
  stacking[cn - 1] = c;
  for(i = 0; i < cn; i++)
    if(stacking[i]->desktop == desktop || stacking[i]->desktop == STICKY)
      client_focus(stacking[i]);  
  ewmh_update_stacking();
}

void client_restore(client *c) {
  int i;
  if(c->desktop != ICONS)
    return;
  nicons--;
  client_to_desktop(c, desktop);
  client_raise(c);
  set_wm_state(c->window, NormalState);
  if(c == current)
    XSetInputFocus(dpy, c->window, RevertToPointerRoot, CurrentTime);
}

void client_save(client *c) {
  int x = c->x, y = c->y;
  if(c->x >= display_width)
    x = display_width - client_width_total(c);
  if(c->y >= display_height)
    y = display_height - client_height_total(c);
  if(c->x + client_width_total(c) <= 0)
    x = 0;
  if(c->y + client_height_total(c) <= 0)
    y = 0;
  client_move(c, x, y);
}

