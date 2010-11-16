#include "matwm.h"

int drag_mode, drag_button, drag_xo, drag_yo;

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
  if(current->desktop == ICONS)
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

int drag_handle_event(XEvent ev) {
  switch(ev.type) {
    case MotionNotify:
      while(XCheckTypedEvent(dpy, MotionNotify, &ev));
      if(drag_mode == RESIZE)
        client_resize(current, snaph(current, ev.xmotion.x, snapv(current, ev.xmotion.x, ev.xmotion.y)) - drag_xo, snapv(current, snaph(current, ev.xmotion.x, ev.xmotion.y), ev.xmotion.y) - drag_yo);
      else if(ev.xmotion.x == display_width - 1 && desktop < dc - 1) {
        client_move(current, -(drag_xo + 1), ev.xmotion.y - drag_yo);
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, 1, ev.xmotion.y);
        desktop_goto(desktop + 1);
      } else if(ev.xmotion.x == 0 && desktop > 0) {
        client_move(current, display_width - (drag_xo + 2), ev.xmotion.y - drag_yo);
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, display_width - 2, ev.xmotion.y);
        desktop_goto(desktop - 1);
      } else client_move(current, snapx(current, ev.xmotion.x - drag_xo, snapy(current, ev.xmotion.x - drag_xo, ev.xmotion.y - drag_yo)), snapy(current, snapx(current, ev.xmotion.x - drag_xo, ev.xmotion.y - drag_yo), ev.xmotion.y - drag_yo));
      return 1;
    case ButtonRelease:
      if(ev.xbutton.button == drag_button || drag_button == AnyButton)
        drag_end();
      return 1;
    case EnterNotify:
    case ButtonPress:
      return 1;
    case UnmapNotify:
      if(current->window == ev.xunmap.window)
        evh = drag_release_wait;
      break;
    case DestroyNotify:
      if(current->window == ev.xdestroywindow.window)
        evh = drag_release_wait;
  }
  return 0;
}

int drag_release_wait(XEvent ev) {
  if(ev.type == ButtonRelease && (ev.xbutton.button == drag_button || drag_button == AnyButton)) {
    drag_end();
    return 1;
  }
  return 0;
}

int snapx(client *c, int nx, int ny) {
  int i;
  if(nx < 0 + snapat && nx > 0 - snapat)
    return 0;
  if(nx < (display_width - client_width_total(c)) + snapat && nx > (display_width - client_width_total(c)) - snapat)
    return display_width - client_width_total(c);
  for(i = 0; i < cn; i++) {
    if(clients[i] == c || (clients[i]->desktop != STICKY && clients[i]->desktop != desktop) || ny + client_height_total(c) < client_y(clients[i]) || ny > client_y(clients[i]) + client_height_total(clients[i]))
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
  int i;
  if(ny < 0 + snapat && ny > 0 - snapat)
    return 0;
  if(ny < (display_height - client_height_total(c)) + snapat && ny > (display_height - client_height_total(c)) - snapat)
    return display_height - client_height_total(c);
  for(i = 0; i < cn; i++) {
    if(clients[i] == c ||(clients[i]->desktop != STICKY && clients[i]->desktop != desktop) || nx + client_width_total(c) < client_x(clients[i]) || nx > client_x(clients[i]) + client_width_total(clients[i]))
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
  int i;
  if(nx < display_width + snapat && nx > display_width - snapat)
    return display_width;
  for(i = 0; i < cn; i++) {
    if(clients[i] == c || (clients[i]->desktop != STICKY && clients[i]->desktop != desktop) || ny < client_y(clients[i]) || c->y > client_y(clients[i]) + client_height_total(clients[i]))
      continue;
    if(nx < client_x(clients[i]) + snapat && nx > client_x(clients[i]) - snapat)
      return client_x(clients[i]);
    if(nx < client_x(clients[i]) + client_width_total(clients[i]) + snapat && nx > client_x(clients[i]) + client_width_total(clients[i]) - snapat)
      return client_x(clients[i]) + client_width_total(clients[i]);
  }
  return nx;
}

int snapv(client *c, int nx, int ny) {
  int i;
  if(ny < display_height + snapat && ny > display_height - snapat)
    return display_height;
  for(i = 0; i < cn; i++) {
    if(clients[i] == c ||  (clients[i]->desktop != STICKY && clients[i]->desktop != desktop) || nx < client_x(clients[i]) || c->x > client_x(clients[i]) + client_width_total(clients[i]))
      continue;
    if(ny < client_y(clients[i]) + snapat && ny > client_y(clients[i]) - snapat)
      return client_y(clients[i]);
    if(ny < client_y(clients[i]) + client_height_total(clients[i]) + snapat && ny > client_y(clients[i]) + client_height_total(clients[i]) - snapat)
      return client_y(clients[i]) + client_height_total(clients[i]);
  }
  return ny;
}

