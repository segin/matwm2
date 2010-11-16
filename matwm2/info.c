#include "matwm.h"

int client_x(client *c) { /* returns the horizontal position of a window's frame */
	if(c->flags & FULLSCREEN)
		return screens[c->screen].x;
	if(c->flags & MAXIMIZED_L)
		return screens[c->screen].x + screens[c->screen].ewmh_strut[0];
	if(c->flags & EXPANDED_L)
		return c->expand_x;
	return c->x;
}

int client_y(client *c) { /* returns the vertical position of a window's frame */
	if(c->flags & FULLSCREEN)
		return screens[c->screen].y;
	if(c->flags & MAXIMIZED_T)
		return screens[c->screen].y + screens[c->screen].ewmh_strut[2];
	if(c->flags & EXPANDED_T)
		return c->expand_y;
	return c->y;
}

int client_width(client *c) { /* returns the width of a client (excluding the frame) */
	if(c->flags & FULLSCREEN)
		return screens[c->screen].width;
	if(c->flags & MAXIMIZED_R)
		return screens[c->screen].width - ((client_border(c) * 2) + screens[c->screen].ewmh_strut[1] + (client_x(c) - screens[c->screen].x));
	if(c->flags & EXPANDED_R)
		return c->expand_width;
	return c->width;
}

int client_height(client *c) {  /* returns the height of a client (excluding the frame) */
	if(c->flags & FULLSCREEN)
		return screens[c->screen].height;
	if(c->flags & MAXIMIZED_B)
		return screens[c->screen].height - ((client_border(c) * 2) + client_title(c) + screens[c->screen].ewmh_strut[3] + (client_y(c) - screens[c->screen].y));
	if(c->flags & EXPANDED_B)
		return c->expand_height;
	return c->height;
}

int client_border(client *c) { /* returns the total width of a client's border (including the border placed around the parent window by X) */
	if(c->flags & FULLSCREEN)
		return 0;
	return (!(c->flags & SHAPED) && c->flags & HAS_BORDER) ? border_spacing + border_width : 0;
}

int client_border_intern(client *c) { /* returns the width of a client's border (excluding the border placed around the parent window by X) */
	if(c->flags & FULLSCREEN)
		return 0;
	return (!(c->flags & SHAPED) && c->flags & HAS_BORDER) ? border_spacing : 0;
}

int client_title(client *c) { /* returns the height of a client's title bar (border not included) */
	if(c->flags & FULLSCREEN)
		return 0;
	return (!(c->flags & SHAPED) && c->flags & HAS_TITLE && c->flags & HAS_BORDER) ? title_height : 0;
}

int client_width_total(client *c) { /* returns the total width of a client (including borders) */
	return client_width(c) + (client_border(c) * 2);
}

int client_height_total(client *c) { /* returns the total width of a client (including borders) */
	return client_height(c) + (client_border(c) * 2) + client_title(c);
}

int client_width_total_intern(client *c) { /* returns the total width of a client minus the 1px border of the parent window */
	return client_width(c) + (client_border_intern(c) * 2);
}

int client_height_total_intern(client *c) { /* returns the total height of a client minus the 1px border of the parent window */
	return client_height(c) + (client_border_intern(c) * 2) + client_title(c);
}

int client_title_width(client *c) { /* returns the width of the title window of a client */
	int avail = client_width(c) - ((c->flags & HAS_BUTTONS) ? (c->buttons_left_width ? c->buttons_left_width + button_spacing : 0) + (c->buttons_right_width ? c->buttons_right_width + button_spacing : 0) : 0);
	return (avail < c->title_width) ? avail : c->title_width;
}

int client_title_x(client *c) { /* returns the horizontal position of the client's title window */
	int center;
	if(center_title) {
		center = (client_border_intern(c) + ((c->flags & HAS_BUTTONS) ? c->buttons_left_width : 0) + ((client_width(c) - ((c->flags & HAS_BUTTONS) ? c->buttons_left_width + c->buttons_right_width : 0)) / 2)) - (c->title_width / 2);
		if(center > border_spacing + ((c->flags & HAS_BUTTONS) ? c->buttons_left_width + button_spacing : 0))
			return center;
	}
	return border_spacing + ((c->flags & HAS_BUTTONS) ? c->buttons_left_width + button_spacing : 0);
}

bool client_visible(client *c) { /* to know if the client is visible on the current desktop */
	if(c) /* so we could call this without first checking if the client exist (we do) */
		if((c->desktop == desktop || c->desktop == STICKY) && !(c->flags & ICONIC))
			return true;
	return false;
}

int client_layer(client *c) { /* returns the "layer" a client is on ("layers" are used for always-on-top and always-below) */
	if(fullscreen_stacking == FS_ALWAYS_ONTOP && c->layer <= NORMAL && c->flags & FULLSCREEN)
		return TOP;
	return c->layer;
}

int client_edge(client *c) {
	if(c->flags & FULLSCREEN)
		return 0;
	return (!(c->flags & SHAPED) && c->flags & HAS_BORDER) ? border_width : 0;
}

int client_number(client **array, client *c) { /* looks wich item in array c is */
	int i;
	for(i = 0; i < cn; i++)
		if(array[i] == c)
			break;
	return i;
}

client *owner(Window w) { /* to know wich client owns the window, if one does */
	int i;
	for(i = 0; i < cn; i++)
		if(clients[i]->parent == w || clients[i]->window == w || clients[i]->wlist_item == w || clients[i]->title == w || clients[i]->button_parent_right == w || clients[i]->button_parent_left == w)
			return clients[i];
	return NULL;
}
