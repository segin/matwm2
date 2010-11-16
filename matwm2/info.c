#include "matwm.h"

int client_x(client *c) {
  if(c->flags & FULLSCREEN)
    return -client_border(c);
  if(c->flags & MAXIMISED)
    return ewmh_strut[0];
  if(c->flags & EXPANDED)
    return c->expand_x;
  return c->x;
}

int client_y(client *c) {
  if(c->flags & FULLSCREEN)
    return -(client_border(c) + client_title(c));
  if(c->flags & MAXIMISED)
    return ewmh_strut[2];
  if(c->flags & EXPANDED)
    return c->expand_y;
  return c->y;
}

int client_width(client *c) {
  if(c->flags & FULLSCREEN)
    return display_width;
  if(c->flags & MAXIMISED)
    return display_width - ((client_border(c) * 2) + ewmh_strut[0] + ewmh_strut[1]);
  if(c->flags & EXPANDED)
    return c->expand_width;
  return c->width;
}

int client_height(client *c) {
  if(c->flags & FULLSCREEN)
    return display_height;
  if(c->flags & MAXIMISED)
    return display_height - ((client_border(c) * 2) + client_title(c) + ewmh_strut[2] + ewmh_strut[3]);
  if(c->flags & EXPANDED)
    return c->expand_height;
  return c->height;
}

int client_border(client *c) {
  return (!(c->flags & SHAPED) && c->flags & HAS_BORDER) ? border_width : 0;
}

int client_border_intern(client *c) {
  return (!(c->flags & SHAPED) && c->flags & HAS_BORDER) ? border_width - 1 : 0;
}

int client_title(client *c) {
  return (!(c->flags & SHAPED) && c->flags & HAS_TITLE && c->flags & HAS_BORDER) ? title_height : 0;
}

int client_width_total(client *c) {
  return client_width(c) + (client_border(c) * 2);
}

int client_height_total(client *c) {
  return client_height(c) + (client_border(c) * 2) + client_title(c);
}

int client_width_total_intern(client *c) {
  return client_width(c) + (client_border_intern(c) * 2);
}

int client_height_total_intern(client *c) {
  return client_height(c) + (client_border_intern(c) * 2) + client_title(c);
}

int title_width(client *c) {
  int avail = client_width(c) - ((c->flags & HAS_BUTTONS) ? button_parent_width + 2 : 0);
  return (avail < c->title_width) ? avail : c->title_width;
}

int client_number(client **array, client *c) {
  int i;
  for(i = 0; i < cn; i++)
    if(array[i] == c)
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

