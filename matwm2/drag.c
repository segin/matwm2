#include "matwm.h"

int drag_mode, drag_xo, drag_yo;
unsigned int drag_button;

void drag_start(int mode, int button, int x, int y) {
	if(evh)
		return;
	if(mode == RESIZE) {
		if(!(current->flags & CAN_RESIZE))
			return;
		client_warp(current);
		drag_xo = client_x(current) + (client_border(current) * 2);
		drag_yo = client_y(current) + (client_border(current) * 2) + client_title(current);
	} else {
		if(!(current->flags & CAN_MOVE))
			return;
		drag_xo = x - client_x(current);
		drag_yo = y - client_y(current);
	}
	if(current->flags & ICONIC)
		client_restore(current);
	drag_mode = mode;
	drag_button = button;
	evh = drag_handle_event;
	client_raise(current);
	XGrabPointer(dpy, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, 0, CurrentTime);
}

void drag_end(void) {
	XUngrabPointer(dpy, CurrentTime);
	evh = NULL;
}

bool drag_handle_event(XEvent ev) {
	int x, y, left = screens_leftmost(), right = screens_rightmost();
	if(current)
		if(has_child(current->parent, current->window) || ev.type == UnmapNotify || ev.type == DestroyNotify || ev.type == ButtonRelease)
			switch(ev.type) {
				case MotionNotify:
					while(XCheckTypedEvent(dpy, MotionNotify, &ev));
					if(drag_mode == RESIZE) {
						x = ev.xmotion.x + 2;
						y = ev.xmotion.y + 2;
						if(nosnapmodmask && ev.xmotion.state & nosnapmodmask)
							client_resize(current, x - drag_xo, y - drag_yo);
						else client_resize(current, snaph(current, x, snapv(current, x, y)) - drag_xo, snapv(current, snaph(current, x, y), y) - drag_yo);
					} else if(drag_warp && ev.xmotion.x == right - 1 && desktop < dc - 1) {
						client_move(current, left - (drag_xo + 1), ev.xmotion.y - drag_yo);
						XWarpPointer(dpy, None, root, 0, 0, 0, 0, 1, ev.xmotion.y);
						desktop_goto(desktop + 1);
					} else if(drag_warp && ev.xmotion.x == left && desktop > 0) {
						client_move(current, right - (drag_xo + 2), ev.xmotion.y - drag_yo);
						XWarpPointer(dpy, None, root, 0, 0, 0, 0, right - 2, ev.xmotion.y);
						desktop_goto(desktop - 1);
					} else if(nosnapmodmask && ev.xmotion.state & nosnapmodmask)
						client_move(current, ev.xmotion.x - drag_xo, ev.xmotion.y - drag_yo);
					else client_move(current, snapx(current, ev.xmotion.x - drag_xo, snapy(current, ev.xmotion.x - drag_xo, ev.xmotion.y - drag_yo)), snapy(current, snapx(current, ev.xmotion.x - drag_xo, ev.xmotion.y - drag_yo), ev.xmotion.y - drag_yo));
					return true;
				case ButtonRelease:
					if(ev.xbutton.button == drag_button || drag_button == AnyButton)
						drag_end();
					return true;
				case EnterNotify:
				case ButtonPress:
					return true;
				case UnmapNotify:
					if(current->window == ev.xunmap.window)
						evh = drag_release_wait;
					break;
				case DestroyNotify:
					if(current->window == ev.xdestroywindow.window)
						evh = drag_release_wait;
			}
	return false;
}

bool drag_release_wait(XEvent ev) {
	if(ev.type == ButtonRelease && (ev.xbutton.button == drag_button || drag_button == AnyButton)) {
		drag_end();
		return true;
	}
	return false;
}

int snapx(client *c, int nx, int ny) {
	int i, right;
	for(i = 0; i < nscreens; i++) {
		if(nx < screens[i].x + snapat && nx > screens[i].x - snapat)
			return screens[i].x;
		right = screens[i].x + screens[i].width;
		if(nx < (right - client_width_total(c)) + snapat && nx > (right - client_width_total(c)) - snapat)
			return right - client_width_total(c);
	}
	for(i = 0; i < cn; i++) {
		if(clients[i] == c || !client_visible(clients[i]) || ny + client_height_total(c) < client_y(clients[i]) || ny > client_y(clients[i]) + client_height_total(clients[i]))
			continue;
		if(nx < client_x(clients[i]) + snapat && nx > client_x(clients[i]) - snapat)
			return client_x(clients[i]);
		if(nx < client_x(clients[i]) + client_width_total(clients[i]) + snapat && nx > client_x(clients[i]) + client_width_total(clients[i]) - snapat)
			return client_x(clients[i]) + client_width_total(clients[i]);
		if(nx + client_width_total(c) < client_x(clients[i]) + snapat && nx + client_width_total(c) > client_x(clients[i]) - snapat)
			return client_x(clients[i]) - client_width_total(c);
		if(nx + client_width_total(c) < client_x(clients[i]) + client_width_total(clients[i]) + snapat && nx + client_width_total(c) > client_x(clients[i]) + client_width_total(clients[i]) - snapat)
			return client_x(clients[i]) + client_width_total(clients[i]) - client_width_total(c);
	}
	return nx;
}

int snapy(client *c, int nx, int ny) {
	int i, bottom;
	for(i = 0; i < nscreens; i++) {
		if(ny < screens[i].y + snapat && ny > screens[i].y - snapat)
			return 0;
		bottom = screens[i].y + screens[i].height;
		if(ny < (bottom - client_height_total(c)) + snapat && ny > (bottom - client_height_total(c)) - snapat)
			return bottom - client_height_total(c);
	}
	for(i = 0; i < cn; i++) {
		if(clients[i] == c || !client_visible(clients[i]) || nx + client_width_total(c) < client_x(clients[i]) || nx > client_x(clients[i]) + client_width_total(clients[i]))
			continue;
		if(ny < client_y(clients[i]) + snapat && ny > client_y(clients[i]) - snapat)
			return client_y(clients[i]);
		if(ny < client_y(clients[i]) + client_height_total(clients[i]) + snapat && ny > client_y(clients[i]) + client_height_total(clients[i]) - snapat)
			return client_y(clients[i]) + client_height_total(clients[i]);
		if(ny + client_height_total(c) < client_y(clients[i]) + snapat && ny + client_height_total(c) > client_y(clients[i]) - snapat)
			return client_y(clients[i]) - client_height_total(c);
		if(ny + client_height_total(c) < client_y(clients[i]) + client_height_total(clients[i]) + snapat && ny + client_height_total(c) > client_y(clients[i]) + client_height_total(clients[i]) - snapat)
			return client_y(clients[i]) + client_height_total(clients[i]) - client_height_total(c);
	}
	return ny;
}

int snaph(client *c, int nx, int ny) {
	int i, right;
	for(i = 0; i < nscreens; i++) {
		right = screens[i].x + screens[i].width;
		if(nx < right + snapat && nx > right - snapat)
			return right;
	}
	for(i = 0; i < cn; i++) {
		if(clients[i] == c || (clients[i]->desktop != STICKY && clients[i]->desktop != desktop) || ny < client_y(clients[i]) || c->y > client_y(clients[i]) + client_height_total(clients[i]) || clients[i]->flags & ICONIC)
			continue;
		if(nx < client_x(clients[i]) + snapat && nx > client_x(clients[i]) - snapat)
			return client_x(clients[i]);
		if(nx < client_x(clients[i]) + client_width_total(clients[i]) + snapat && nx > client_x(clients[i]) + client_width_total(clients[i]) - snapat)
			return client_x(clients[i]) + client_width_total(clients[i]);
	}
	return nx;
}

int snapv(client *c, int nx, int ny) {
	int i, bottom;
	for(i = 0; i < nscreens; i++) {
		bottom = screens[i].y + screens[i].height;
		if(ny < bottom + snapat && ny > bottom - snapat)
			return bottom;
	}
	for(i = 0; i < cn; i++) {
		if(clients[i] == c ||	(clients[i]->desktop != STICKY && clients[i]->desktop != desktop) || nx < client_x(clients[i]) || c->x > client_x(clients[i]) + client_width_total(clients[i]) || clients[i]->flags & ICONIC)
			continue;
		if(ny < client_y(clients[i]) + snapat && ny > client_y(clients[i]) - snapat)
			return client_y(clients[i]);
		if(ny < client_y(clients[i]) + client_height_total(clients[i]) + snapat && ny > client_y(clients[i]) + client_height_total(clients[i]) - snapat)
			return client_y(clients[i]) + client_height_total(clients[i]);
	}
	return ny;
}

