#include "matwm.h"

int client_x(client *c) {
	if(c->flags & FULLSCREEN)
		return -client_border(c);
	if(c->flags & MAXIMIZED_L)
		return ewmh_strut[0];
	if(c->flags & EXPANDED_L)
		return c->expand_x;
	return c->x;
}

int client_y(client *c) {
	if(c->flags & FULLSCREEN)
		return -(client_border(c) + client_title(c));
	if(c->flags & MAXIMIZED_T)
		return ewmh_strut[2];
	if(c->flags & EXPANDED_T)
		return c->expand_y;
	return c->y;
}

int client_width(client *c) {
	if(c->flags & FULLSCREEN)
		return display_width;
	if(c->flags & MAXIMIZED_R)
		return display_width - ((client_border(c) * 2) + ewmh_strut[1] + client_x(c));
	if(c->flags & EXPANDED_R)
		return c->expand_width;
	return c->width;
}

int client_height(client *c) {
	if(c->flags & FULLSCREEN)
		return display_height;
	if(c->flags & MAXIMIZED_B)
		return display_height - ((client_border(c) * 2) + client_title(c) + ewmh_strut[3] + client_y(c));
	if(c->flags & EXPANDED_B)
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

int client_title_width(client *c) {
	int avail = client_width(c) - ((c->flags & HAS_BUTTONS) ? c->buttons_left_width + c->buttons_right_width : 0);
	return (avail < c->title_width) ? avail : c->title_width;
}

int client_title_y(client *c) {
	int center;
	if(center_title) {
		center = (client_border_intern(c) + ((c->flags & HAS_BUTTONS) ? c->buttons_left_width : 0) + ((client_width(c) - ((c->flags & HAS_BUTTONS) ? c->buttons_left_width + c->buttons_right_width : 0)) / 2)) - (c->title_width / 2);
		if(center > border_width + ((c->flags & HAS_BUTTONS) ? c->buttons_left_width : 0) - 1)
			return center;
	}
	return border_width + ((c->flags & HAS_BUTTONS) ? c->buttons_left_width : 0) - 1;
}

int client_visible(client *c) {
	return (c->desktop == desktop || c->desktop == STICKY) && !(c->flags & ICONIC);
}

int client_layer(client *c) {
	if(c->layer <= NORMAL && c->flags & FULLSCREEN)
		return TOP;
	return c->layer;
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
		if(clients[i]->parent == w || clients[i]->window == w || clients[i]->wlist_item == w || clients[i]->title == w || clients[i]->button_parent_right == w || clients[i]->button_parent_left == w)
			return clients[i];
	return NULL;
}

