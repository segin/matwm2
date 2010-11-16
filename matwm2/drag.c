#include "matwm.h"

int drag_xo, drag_yo, xr, yr;
unsigned int drag_button;
unsigned char drag_mode;

void drag_start(unsigned char mode, int button, int x, int y) {
	#ifdef DEBUG
	printf(NAME ": drag_start(): entering drag mode\n");
	#endif
	if(evh)
		return;
	if(mode == A_RESIZE) {
		if(!(current->flags & CAN_RESIZE))
			return;
		client_warp(current);
		xr = client_x(current);
		yr = client_y(current);
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
	#ifdef DEBUG
	printf(NAME ": drag_end(): ending drag mode\n");
	#endif
	XUngrabPointer(dpy, CurrentTime);
	evh = NULL;
}

bool drag_handle_event(XEvent *ev) {
	int x, y, left = screens_leftmost(), right = screens_rightmost();
	if(current)
		if(has_child(current->parent, current->window) || ev->type == UnmapNotify || ev->type == DestroyNotify || ev->type == ButtonRelease)
			switch(ev->type) {
				case MotionNotify:
					while(XCheckTypedEvent(dpy, MotionNotify, ev));
					if(drag_mode == A_RESIZE) {
						x = ev->xmotion.x + 2;
						y = ev->xmotion.y + 2;
						if(nosnapmodmask && ev->xmotion.state & nosnapmodmask)
							client_resize(current, x - drag_xo, y - drag_yo);
						else client_resize(current, snap(current, x, snap(current, x, y, 'v'), 'h') - drag_xo, snap(current, snap(current, x, y, 'h'), y, 'v') - drag_yo);
					} else if(drag_warp && ev->xmotion.x == right - 1 && desktop < dc - 1) {
						client_move(current, left - (drag_xo + 1), ev->xmotion.y - drag_yo);
						XWarpPointer(dpy, None, root, 0, 0, 0, 0, 1, ev->xmotion.y);
						desktop_goto(desktop + 1);
					} else if(drag_warp && ev->xmotion.x == left && desktop > 0) {
						client_move(current, right - (drag_xo + 2), ev->xmotion.y - drag_yo);
						XWarpPointer(dpy, None, root, 0, 0, 0, 0, right - 2, ev->xmotion.y);
						desktop_goto(desktop - 1);
					} else if(nosnapmodmask && ev->xmotion.state & nosnapmodmask)
						client_move(current, ev->xmotion.x - drag_xo, ev->xmotion.y - drag_yo);
					else client_move(current, snap(current, ev->xmotion.x - drag_xo, snap(current, ev->xmotion.x - drag_xo, ev->xmotion.y - drag_yo, 'v'), 'h'), snap(current, snap(current, ev->xmotion.x - drag_xo, ev->xmotion.y - drag_yo, 'h'), ev->xmotion.y - drag_yo, 'v'));
					return true;
				case ButtonRelease:
					if(ev->xbutton.button == drag_button || drag_button == AnyButton) {
						#ifdef DEBUG_EVENTS
						printf(NAME ": drag_handle_event(): handling ButtonRelease event\n");
						#endif
						drag_end();
					}
					return true;
				case EnterNotify:
				case ButtonPress:
					return true;
				case UnmapNotify:
					if(current->window == ev->xunmap.window) {
						#ifdef DEBUG_EVENTS
						printf(NAME ": drag_handle_event(): got UnmapNotify\n");
						#endif
						evh = drag_release_wait;
					}
					break;
				case DestroyNotify:
					if(current->window == ev->xdestroywindow.window) {
						#ifdef DEBUG_EVENTS
						printf(NAME ": drag_handle_event(): got DestroyNotify\n");
						#endif
						evh = drag_release_wait;
					}
			}
	return false;
}

bool drag_release_wait(XEvent *ev) {
	if(ev->type == ButtonRelease && (ev->xbutton.button == drag_button || drag_button == AnyButton)) {
		#ifdef DEBUG_EVENTS
		printf(NAME ": drag_release_wait(): handling ButtonRelease event\n");
		#endif
		drag_end();
		return true;
	}
	return false;
}

bool __snap(int x1, int x2, int *ret) {
	if(x1 < x2 + snapat && x1 > x2 - snapat) {
		*ret = x2;
		return true;
	}
	return false;
}

bool _snap(int r, int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, int *ret) {
	if(drag_mode == A_MOVE && y1 <= y2 + h2 && y1 + h1 >= y2)
		if(__snap(x1, x2, ret) || __snap(x1, x2 - w1, ret) || __snap(x1, x2 + w2, ret) || __snap(x1, x2 + w2 - w1, ret))
			return true;
	if(drag_mode == A_RESIZE && r <= y2 + h2 && r + h1 >= y2)
		if(__snap(x1, x2, ret) || __snap(x1, x2 + w2, ret))
			return true;
	return false;
}

int snap(client *c, int nx, int ny, char axis) {
	int i, ret, width, height;
	width = client_width_total(c);
	height = client_height_total(c);
	if(axis == 'h') {
		for(i = 0; i < nscreens; i++)
			if(_snap(yr, nx, ny, width, height, screens[i].x, screens[i].y, screens[i].width, screens[i].height, &ret))
				return ret;
		for(i = 0; i < cn; i++)
			if(stacking[i] != c && client_visible(stacking[i]) && _snap(yr, nx, ny, width, height, client_x(stacking[i]), client_y(stacking[i]), client_width_total(stacking[i]), client_height_total(stacking[i]), &ret))
				return ret;
	} else {
		for(i = 0; i < nscreens; i++)
			if(_snap(xr, ny, nx, height, width, screens[i].y, screens[i].x, screens[i].height, screens[i].width, &ret))
				return ret;
		for(i = 0; i < cn; i++)
			if(stacking[i] != c && client_visible(stacking[i]) && _snap(xr, ny, nx, height, width, client_y(stacking[i]), client_x(stacking[i]), client_height_total(stacking[i]), client_width_total(stacking[i]), &ret))
				return ret;
	}
	return (axis == 'h') ? nx : ny;
}
