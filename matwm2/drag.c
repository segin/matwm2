#include "matwm.h"

XButtonEvent be;
int xo, yo;

Bool isrelease(Display *display, XEvent *event, XPointer arg) {
}

void drag_start(XEvent ev) {
  if(evh)
    return;
  evh = drag_handle_event;
  be = ev.xbutton;
  raise_client(current);
  if(buttonaction(be.button) == BA_RESIZE) {
    warp();
    xo = current->x + (border(current) * 2);
    yo = current->y + (border(current) * 2) + title(current);
  }
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
      if(buttonaction(be.button) == BA_RESIZE) {
        resize(current, snaph(current, ev.xmotion.x, snapv(current, ev.xmotion.x, ev.xmotion.y)) - xo, snapv(current, snaph(current, ev.xmotion.x, ev.xmotion.y), ev.xmotion.y) - yo);
      } else move(current, snapx(current, ev.xmotion.x - be.x, snapy(current, ev.xmotion.x - be.x, ev.xmotion.y - be.y)), snapy(current, snapx(current, ev.xmotion.x - be.x, ev.xmotion.y - be.y), ev.xmotion.y - be.y));
      return 1;
    case ButtonRelease:
      if(ev.xbutton.button == be.button)
        drag_end();
    case EnterNotify:
    case ButtonPress:
      return 1;
    case UnmapNotify:
      if(current->window == ev.xunmap.window) {
        remove_client(current, 1);
        drag_end();
        evh = drag_release_wait;
        return 1;
      }
      break;
    case DestroyNotify:
      if(current->window == ev.xdestroywindow.window) {
        remove_client(current, 2);
        drag_end();
        evh = drag_release_wait;
        return 1;
      }
  }
  return 0;
}

int drag_release_wait(XEvent ev) {
  if(ev.type == ButtonRelease && ev.xbutton.button == be.button) {
    evh = NULL;
    return 1;
  }
  return 0;
}

int snapx(client *c, int nx, int ny) {
  int i;
  if(nx < 0 + snapat && nx > 0 - snapat)
    return 0;
  if(nx < (display_width - total_width(c)) + snapat && nx > (display_width - total_width(c)) - snapat)
    return display_width - total_width(c);
  for(i = 0; i < cn; i++) {
    if(clients[i] == c || clients[i]->iconic || ny + total_height(c) < clients[i]->y || ny > clients[i]->y + total_height(clients[i]))
      continue;
    if(nx < clients[i]->x + snapat && nx > clients[i]->x - snapat)
      return clients[i]->x;
    if(nx < clients[i]->x + total_width(clients[i]) + snapat && nx > clients[i]->x + total_width(clients[i]) - snapat)
      return clients[i]->x + total_width(clients[i]);
    if(nx + total_width(c) < clients[i]->x + snapat && nx + total_width(c) > clients[i]->x - snapat)
      return clients[i]->x - total_width(c);
    if(nx + total_width(c) < clients[i]->x + total_width(clients[i]) + snapat && nx + total_width(c) > clients[i]->x + total_width(clients[i]) - snapat)
      return clients[i]->x + total_width(clients[i]) - total_width(c);
  }
  return nx;
}

int snapy(client *c, int nx, int ny) {
  int i;
  if(ny < 0 + snapat && ny > 0 - snapat)
    return 0;
  if(ny < (display_height - total_height(c)) + snapat && ny > (display_height - total_height(c)) - snapat)
    return display_height - total_height(c);
  for(i = 0; i < cn; i++) {
    if(clients[i] == c || clients[i]->iconic || nx + total_width(c) < clients[i]->x || nx > clients[i]->x + total_width(clients[i]))
      continue;
    if(ny < clients[i]->y + snapat && ny > clients[i]->y - snapat)
      return clients[i]->y;
    if(ny < clients[i]->y + total_height(clients[i]) + snapat && ny > clients[i]->y + total_height(clients[i]) - snapat)
      return clients[i]->y + total_height(clients[i]);
    if(ny + total_height(c) < clients[i]->y + snapat && ny + total_height(c) > clients[i]->y - snapat)
      return clients[i]->y - total_height(c);
    if(ny + total_height(c) < clients[i]->y + total_height(clients[i]) + snapat && ny + total_height(c) > clients[i]->y + total_height(clients[i]) - snapat)
      return clients[i]->y + total_height(clients[i]) - total_height(c);
  }
  return ny;
}

int snaph(client *c, int nx, int ny) {
  int i;
  if(nx < display_width + snapat && nx > display_width - snapat)
    return display_width;
  for(i = 0; i < cn; i++) {
    if(clients[i] == c || clients[i]->iconic || ny < clients[i]->y || c->y > clients[i]->y + total_height(clients[i]))
      continue;
    if(nx < clients[i]->x + snapat && nx > clients[i]->x - snapat)
      return clients[i]->x;
    if(nx < clients[i]->x + total_width(clients[i]) + snapat && nx > clients[i]->x + total_width(clients[i]) - snapat)
      return clients[i]->x + total_width(clients[i]);
  }
  return nx;
}

int snapv(client *c, int nx, int ny) {
  int i;
  if(ny < display_height + snapat && ny > display_height - snapat)
    return display_height;
  for(i = 0; i < cn; i++) {
    if(clients[i] == c || clients[i]->iconic || nx < clients[i]->x || c->x > clients[i]->x + total_width(clients[i]))
      continue;
    if(ny < clients[i]->y + snapat && ny > clients[i]->y - snapat)
      return clients[i]->y;
    if(ny < clients[i]->y + total_height(clients[i]) + snapat && ny > clients[i]->y + total_height(clients[i]) - snapat)
      return clients[i]->y + total_height(clients[i]);
  }
  return ny;
}

