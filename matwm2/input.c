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

void ungrab_key(Window w, unsigned int modmask, KeyCode key) {
  XUngrabKey(dpy, key, modmask, w);
  XUngrabKey(dpy, key, LockMask | modmask, w);
  if(numlockmask) {
    XUngrabKey(dpy, key, numlockmask | modmask, w);
  XUngrabKey(dpy, key, numlockmask | LockMask | modmask, w);
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
keybind *evkey(XEvent ev) {
  int i;
  for(i = 0; i < keyn; i++)
    if(keys[i].code == ev.xkey.keycode && cmpmask(ev.xkey.state, keys[i].mask))
      return &keys[i];
}

int keyaction(XEvent ev) {
  int i;
  for(i = 0; i < keyn; i++)
    if(keys[i].code == ev.xkey.keycode && cmpmask(ev.xkey.state, keys[i].mask))
      return keys[i].action;
  return KA_NONE;
}

int key_to_mask(KeyCode key) {
  int i;
  for(i = 0; i < 8 * modmap->max_keypermod; i++)
    if(modmap->modifiermap[i] == key)
      return 1 << (i / modmap->max_keypermod);
  return 0;
}

