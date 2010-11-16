#include "matwm.h"
#include <X11/keysym.h>

unsigned int numlockmask = 0;

void init_input(void) {
  XModifierKeymap *modmap = XGetModifierMapping(dpy);
  int i;
  for(i = 0; i < 8; i++)
    if(modmap->modifiermap[modmap->max_keypermod * i] == XKeysymToKeycode(dpy, XK_Num_Lock))
      numlockmask = (1 << i);
}

void grab_button(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask) {
  XGrabButton(dpy, button, modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  XGrabButton(dpy, button, LockMask|modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  if(numlockmask) {
    XGrabButton(dpy, button, numlockmask|modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
    XGrabButton(dpy, button, numlockmask|LockMask|modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  }
}

void buttonpress(int n, unsigned int button) {
  switch(button) {
    case Button1:
      XRaiseWindow(dpy, clients[n].parent);
      drag(n, 0);
      break;
    case Button2:
      delete_window(n);
      break;
    case Button3:
      XRaiseWindow(dpy, clients[n].parent);
      drag(n, 1);
      break;
    case Button4:
      XRaiseWindow(dpy, clients[n].parent);
      break;
    case Button5:
      XLowerWindow(dpy, clients[n].parent);
    break;
  }
}

void drag(int n, int r) {
  Window dw1, dw2;
  int xo, yo, t1, t2;
  unsigned int t3;
  XEvent ev;
  if(r) {
    XWarpPointer(dpy, None, clients[n].parent, 0, 0, 0, 0, clients[n].width + border_width,  clients[n].height + border_width + title_height);
    xo = clients[n].x + border_width;
    yo = clients[n].y + border_width + title_height;
  } else XQueryPointer(dpy, clients[n].parent, &dw1, &dw2, &t1, &t2, &xo, &yo, &t3);
  XGrabPointer(dpy, root, True, ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, 0, CurrentTime);
  while(1) {
    XMaskEvent(dpy, SubstructureNotifyMask | SubstructureRedirectMask | ButtonReleaseMask | PointerMotionMask | ExposureMask | EnterWindowMask, &ev);
    if(ev.type == MotionNotify) {
      if(r) {
        resize(n, ev.xmotion.x - xo,  ev.xmotion.y - yo);
      } else move(n, ev.xmotion.x - xo, ev.xmotion.y - yo);
    } else if(ev.type == ButtonRelease) {
      break;
    } else if(ev.type == EnterNotify) {
      continue;
    } else if(ev.type == UnmapNotify && ev.xunmap.window == clients[n].window) {
      remove_client(n);
      break;
    } else handle_event(ev);
  }
  XUngrabPointer(dpy, CurrentTime);
}

