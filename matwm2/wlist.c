#include "matwm.h"

Window wlist;
int wlist_width;

void wlist_start(XEvent ev) {
  if(evh || !cn)
    return;
  wlist_width = 3;
  wlist_update();
  XMapRaised(dpy, wlist);
  XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
  evh = wlist_handle_event;
  handle_event(ev);
}

void wlist_end(void) {
  XUngrabKeyboard(dpy, CurrentTime);
  XUnmapWindow(dpy, wlist);
  evh = NULL;
  if(!current)
    return;
  client_save(current);
  if(current->flags & ICONIC)
    client_restore(current);
  else
    client_raise(current);
  client_warp(current);
}

int wlist_handle_event(XEvent ev) {
  int mask, i;
  switch(ev.type) {
    case KeyPress:
      if(keyaction(ev) == KA_NEXT) {
        client_focus((client_number(stacking, current) < cn - 1) ? stacking[client_number(stacking, current) + 1] : stacking[0]);
      } else if(keyaction(ev) == KA_PREV) {
        client_focus((client_number(stacking, current) > 0) ? stacking[client_number(stacking, current) - 1] : stacking[cn - 1]);
      } else break;
      XWarpPointer(dpy, None, current->wlist_item, 0, 0, 0, 0, wlist_width - 2, 3 + title_height);
      break;
    case KeyRelease:
      mask = key_to_mask(ev.xkey.keycode);
      if(mask) {
        mask ^= ev.xkey.state & mask;
        for(i = 0; i < keyn; i++)
          if((keys[i].action == KA_NEXT || keys[i].action == KA_PREV) && cmpmodmask(keys[i].mask, mask))
            break;
        if(i == keyn)
          wlist_end();
      }
    case ButtonPress:
    case ButtonRelease:
      break;
    default:
      return 0;
  }
  return 1;
}

void wlist_update(void) {
  int i, tl, nc = 0, offset = 1;
  if(!cn)
    wlist_end();
  for(i = 0; i < cn; i++)
    if(stacking[i]->name) {
      tl = XTextWidth(font, stacking[i]->name, strlen(stacking[i]->name)) + 6;
      if(tl > wlist_width)
        wlist_width = tl;
    }
  if(wlist_width > display_width)
    wlist_width = display_width;
  for(i = 0; i < cn; i++) {
    if(i == cn - nicons)
      offset = 2;
    XMoveResizeWindow(dpy, stacking[i]->wlist_item, 1, offset + ((title_height + 5) * nc), wlist_width - 2, title_height + 4);
    nc++;
  }
  XMoveResizeWindow(dpy, wlist, (display_width / 2) - (wlist_width / 2), (display_height / 2) - (1 + ((title_height + 5) * cn) / 2), wlist_width, offset + ((title_height + 5) * cn));
}

void wlist_item_draw(client *c) {
  if(c->name)
    XDrawString(dpy, c->wlist_item, (c == current) ? gc : igc, 2, 2 + font->max_bounds.ascent, c->name, strlen(c->name));
}

