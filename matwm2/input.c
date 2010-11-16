#include "matwm.h"

unsigned int mousemodmask = 0, numlockmask = 0;

void grab_key(Window w, unsigned int modmask, KeyCode key) {
  XGrabKey(dpy, key, modmask, w, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(dpy, key, LockMask | modmask, w, True, GrabModeAsync, GrabModeAsync);
  if(numlockmask) {
    XGrabKey(dpy, key, numlockmask | modmask, w, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, key, numlockmask | LockMask | modmask, w, True, GrabModeAsync, GrabModeAsync);
  }
}

void grab_button(Window w, unsigned int button, unsigned int modmask, unsigned int event_mask) {
  XGrabButton(dpy, button, modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  XGrabButton(dpy, button, LockMask | modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  if(numlockmask) {
    XGrabButton(dpy, button, numlockmask | modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
    XGrabButton(dpy, button, numlockmask | LockMask | modmask, w, False, event_mask, GrabModeAsync, GrabModeSync, None, None);
  }
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

void drag(XButtonEvent *be, int res) {
  int xo, yo;
  XEvent ev;
  restack_client(current, 1);
  if(res) {
    warp();
    xo = clients[current].x + border(current);
    yo = clients[current].y + border(current) + title(current);
  }
  XGrabPointer(dpy, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, 0, CurrentTime);
  while(1) {
//    leaving this here just in case there turns out to be a problem with just doing nextevent - wich is now used so also things like shape events are handled
//    XMaskEvent(dpy, PropertyChangeMask | SubstructureNotifyMask | SubstructureRedirectMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask | EnterWindowMask, &ev);
    XNextEvent(dpy, &ev);
    if(ev.type == MotionNotify) {
      while(XCheckTypedEvent(dpy, MotionNotify, &ev));
      if(res) {
        resize(current, ev.xmotion.x - xo,  ev.xmotion.y - yo);
      } else move(current, snapx(current, ev.xmotion.x - be->x, snapy(current, ev.xmotion.x - be->x, ev.xmotion.y - be->y)), snapy(current, snapx(current, ev.xmotion.x - be->x, ev.xmotion.y - be->y), ev.xmotion.y - be->y)); // i schould make a little this more readable, no?
    } else if(ev.type == ButtonRelease && ev.xbutton.button == be->button) {
      break;
    } else if(ev.type == EnterNotify || ev.type == ButtonPress) {
      continue;
    } else  {
      handle_event(ev);
      if((ev.type == UnmapNotify && ev.xunmap.window == clients[current].window) || (ev.type == DestroyNotify && ev.xdestroywindow.window == clients[current].window) || ev.type == KeyPress) {
        while(1) {
          XMaskEvent(dpy, ButtonReleaseMask, &ev);
          if(ev.xbutton.button == be->button)
            break;
        }
        break;
      }
    }
  }
  XUngrabPointer(dpy, CurrentTime);
}

int getmodifier(char *name) {
  if(strcmp(name, "shift") == 0)
    return ShiftMask;
  if(strcmp(name, "lock") == 0)
    return LockMask;
  if(strcmp(name, "control") == 0)
    return ControlMask;
  if(strcmp(name, "mod1") == 0)
    return Mod1Mask;
  if(strcmp(name, "mod2") == 0)
    return Mod2Mask;
  if(strcmp(name, "mod3") == 0)
    return Mod3Mask;
  if(strcmp(name, "mod4") == 0)
    return Mod4Mask;
  if(strcmp(name, "mod5") == 0)
    return Mod5Mask;
  return 0;
}

void mapkeys(void) {
  int i;
  XModifierKeymap *modmap = XGetModifierMapping(dpy);
  for(i = 0; i < 8; i++)
    if(modmap->modifiermap[i * modmap->max_keypermod] == XKeysymToKeycode(dpy, XK_Num_Lock))
      numlockmask = (1 << i);
  string_to_key(xrm_getstr(cfg, "mouse_modifier", DEF_MOUSEMOD), &mousemodmask);
  key_next = xrm_getkey(cfg, "key_next", DEF_KEY_NEXT);
  key_prev = xrm_getkey(cfg, "key_prev", DEF_KEY_PREV);
  key_next_icon = xrm_getkey(cfg, "key_next_icon", DEF_KEY_NEXT_ICON);
  key_prev_icon = xrm_getkey(cfg, "key_prev_icon", DEF_KEY_PREV_ICON);
  key_iconify = xrm_getkey(cfg, "key_iconify", DEF_KEY_ICONIFY);
  key_close = xrm_getkey(cfg, "key_close", DEF_KEY_CLOSE);
  key_maximise = xrm_getkey(cfg, "key_maximise", DEF_KEY_MAXIMISE);
  key_bottomleft = xrm_getkey(cfg, "key_bottomleft", DEF_KEY_BOTTOMLEFT);
  key_bottomright = xrm_getkey(cfg, "key_bottomright", DEF_KEY_BOTTOMRIGHT);
  key_topleft = xrm_getkey(cfg, "key_topleft", DEF_KEY_TOPLEFT);
  key_topright = xrm_getkey(cfg, "key_topright", DEF_KEY_TOPRIGHT);
}

