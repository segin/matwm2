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
  restack_client(current, 1);
  if(buttonaction(be.button) == BA_RESIZE) {
    warp();
    xo = clients[current].x + (border(current) * 2);
    yo = clients[current].y + (border(current) * 2) + title(current);
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
      if(clients[current].window == ev.xunmap.window) {
        remove_client(current, 1);
        drag_end();
        evh = drag_release_wait;
        return 1;
      }
      break;
    case DestroyNotify:
      if(clients[current].window == ev.xdestroywindow.window) {
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

int snapx(int n, int nx, int ny) {
  int i;
  if(nx < 0 + snapat && nx > 0 - snapat)
    return 0;
  if(nx < (display_width - total_width(n)) + snapat && nx > (display_width - total_width(n)) - snapat)
    return display_width - total_width(n);
  for(i = 0; i < cn; i++) {
    if(i == n || clients[i].iconic || ny + total_height(n) < clients[i].y || ny > clients[i].y + total_height(i))
      continue;
    if(nx < clients[i].x + snapat && nx > clients[i].x - snapat)
      return clients[i].x;
    if(nx < clients[i].x + total_width(i) + snapat && nx > clients[i].x + total_width(i) - snapat)
      return clients[i].x + total_width(i);
    if(nx + total_width(n) < clients[i].x + snapat && nx + total_width(n) > clients[i].x - snapat)
      return clients[i].x - total_width(n);
    if(nx + total_width(n) < clients[i].x + total_width(i) + snapat && nx + total_width(n) > clients[i].x + total_width(i) - snapat)
      return clients[i].x + total_width(i) - total_width(n);
  }
  return nx;
}

int snapy(int n, int nx, int ny) {
  int i;
  if(ny < 0 + snapat && ny > 0 - snapat)
    return 0;
  if(ny < (display_height - total_height(n)) + snapat && ny > (display_height - total_height(n)) - snapat)
    return display_height - total_height(n);
  for(i = 0; i < cn; i++) {
    if(i == n || clients[i].iconic || nx + total_width(n) < clients[i].x || nx > clients[i].x + total_width(i))
      continue;
    if(ny < clients[i].y + snapat && ny > clients[i].y - snapat)
      return clients[i].y;
    if(ny < clients[i].y + total_height(i) + snapat && ny > clients[i].y + total_height(i) - snapat)
      return clients[i].y + total_height(i);
    if(ny + total_height(n) < clients[i].y + snapat && ny + total_height(n) > clients[i].y - snapat)
      return clients[i].y - total_height(n);
    if(ny + total_height(n) < clients[i].y + total_height(i) + snapat && ny + total_height(n) > clients[i].y + total_height(i) - snapat)
      return clients[i].y + total_height(i) - total_height(n);
  }
  return ny;
}

int snaph(int n, int nx, int ny) {
  int i;
  if(nx < display_width + snapat && nx > display_width - snapat)
    return display_width;
  for(i = 0; i < cn; i++) {
    if(i == n || clients[i].iconic || ny < clients[i].y || clients[n].y > clients[i].y + total_height(i))
      continue;
    if(nx < clients[i].x + snapat && nx > clients[i].x - snapat)
      return clients[i].x;
    if(nx < clients[i].x + total_width(i) + snapat && nx > clients[i].x + total_width(i) - snapat)
      return clients[i].x + total_width(i);
  }
  return nx;
}

int snapv(int n, int nx, int ny) {
  int i;
  if(ny < display_height + snapat && ny > display_height - snapat)
    return display_height;
  for(i = 0; i < cn; i++) {
    if(i == n || clients[i].iconic || nx < clients[i].x || clients[n].x > clients[i].x + total_width(i))
      continue;
    if(ny < clients[i].y + snapat && ny > clients[i].y - snapat)
      return clients[i].y;
    if(ny < clients[i].y + total_height(i) + snapat && ny > clients[i].y + total_height(i) - snapat)
      return clients[i].y + total_height(i);
  }
  return ny;
}

