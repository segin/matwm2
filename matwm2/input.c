#include "matwm.h"

unsigned int mousemodmask = 0, numlockmask = 0;
XModifierKeymap *modmap;

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
  int i, j;
  modmap = XGetModifierMapping(dpy);
  numlockmask = key_to_mask(XKeysymToKeycode(dpy, XK_Num_Lock));
  string_to_key(xrm_getstr(cfg, "mouse_modifier", DEF_MOUSEMOD), &mousemodmask);
  key_next = xrm_getkey(cfg, "key_next", DEF_KEY_NEXT);
  key_prev = xrm_getkey(cfg, "key_prev", DEF_KEY_PREV);
  key_iconify = xrm_getkey(cfg, "key_iconify", DEF_KEY_ICONIFY);
  key_close = xrm_getkey(cfg, "key_close", DEF_KEY_CLOSE);
  key_maximise = xrm_getkey(cfg, "key_maximise", DEF_KEY_MAXIMISE);
  key_bottomleft = xrm_getkey(cfg, "key_bottomleft", DEF_KEY_BOTTOMLEFT);
  key_bottomright = xrm_getkey(cfg, "key_bottomright", DEF_KEY_BOTTOMRIGHT);
  key_topleft = xrm_getkey(cfg, "key_topleft", DEF_KEY_TOPLEFT);
  key_topright = xrm_getkey(cfg, "key_topright", DEF_KEY_TOPRIGHT);
}

int key_to_mask(KeyCode key) {
  int i;
  for(i = 0; i < 8 * modmap->max_keypermod; i++)
    if(modmap->modifiermap[i] == key)
      return 1 << (i / modmap->max_keypermod);
  return 0;
}

