#include "matwm.h"

int (*evh)(XEvent) = NULL;

void handle_event(XEvent ev) {
  client *c = owner(ev.xany.window);
  int i;
#ifdef DEBUG_EVENTS
  if(c) printf("%s: %s\n", c->name, event_name(ev));
  else printf("%i: %s\n", ev.xany.window, event_name(ev));
#endif
  if(evh && evh(ev))
    return;
  if(button_handle_event(ev) || ewmh_handle_event(ev))
    return;
  if(c && !has_child(c->parent, c->window) && ev.type != DestroyNotify)
    return;
  switch(ev.type) {
    case MapRequest:
      c = owner(ev.xmaprequest.window);
      if(c) {
        if(c->flags & ICONIC && has_child(c->parent, c->window))
          client_restore(c);
      } else if(has_child(root, ev.xmaprequest.window)) {
        client_add(ev.xmaprequest.window);
        ewmh_update_clist();
      }
      break;
    case DestroyNotify:
      c = owner(ev.xdestroywindow.window);
      if(c && c->window == ev.xdestroywindow.window) {
        client_remove(c);
        ewmh_update_clist();
      }
      break;
    case UnmapNotify:
      if(c && c->window == ev.xunmap.window) {
        client_deparent(c);
        set_wm_state(c->window, WithdrawnState);
        client_remove(c);
        ewmh_update_clist();
      }
      break;
    case ConfigureRequest:
      c = owner(ev.xconfigurerequest.window);
      if(c) {
        if(!has_child(c->parent, c->window))
          break;
        if(ev.xconfigurerequest.value_mask & CWX)
          c->x = ev.xconfigurerequest.x - gxo(c, 0);
        if(ev.xconfigurerequest.value_mask & CWY)
          c->y = ev.xconfigurerequest.y - gyo(c, 0);
        if(ev.xconfigurerequest.value_mask & CWWidth)
          c->width = ev.xconfigurerequest.width;
        if(ev.xconfigurerequest.value_mask & CWHeight)
          c->height = ev.xconfigurerequest.height;
        client_update(c);
      } else if(has_child(root, ev.xconfigurerequest.window)) {
        XWindowChanges wc;
        wc.sibling = ev.xconfigurerequest.above;
        wc.stack_mode = ev.xconfigurerequest.detail;
        wc.x = ev.xconfigurerequest.x;
        wc.y = ev.xconfigurerequest.y;
        wc.width = ev.xconfigurerequest.width;
        wc.height = ev.xconfigurerequest.height;
        XConfigureWindow(dpy, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &wc);
      }
      break;
    case PropertyNotify:
     if(c && ev.xproperty.atom == XA_WM_NAME) {
        if(c->name != no_title)
          XFree(c->name);
        XFreePixmap(dpy, c->title_pixmap);
        XFetchName(dpy, c->window, &c->name);
        client_update_name(c);
        XClearWindow(dpy, c->title);
        if(evh == wlist_handle_event) {
          XClearWindow(dpy, c->wlist_item);
          wlist_item_draw(c);
        }
      }
      if(ev.xproperty.atom == XA_WM_NORMAL_HINTS && c)
        get_normal_hints(c);
      break;
    case ClientMessage:
      if(c && ev.xclient.message_type == xa_wm_change_state && ev.xclient.data.l[0] == IconicState)
        client_iconify(c);
      break;
    case EnterNotify:
      if(c && c != current)
        client_focus(c);
      break;
    case Expose:
      if(ev.xexpose.count == 0 && evh == wlist_handle_event && c && ev.xexpose.window == c->wlist_item)
        wlist_item_draw(c);
      break;
    case ButtonPress:
      if(c && buttonaction(ev.xbutton.button) == BA_MOVE)
        drag_start(MOVE, ev.xbutton.button, ev.xbutton.x_root, ev.xbutton.y_root);
      if(c && buttonaction(ev.xbutton.button) == BA_RESIZE)
        drag_start(RESIZE, ev.xbutton.button, ev.xbutton.x_root, ev.xbutton.y_root);
      break;
    case ButtonRelease:
      if(c) {
        if(buttonaction(ev.xbutton.button) == BA_RAISE)
          client_raise(c);
        if(buttonaction(ev.xbutton.button) == BA_LOWER)
          client_lower(c);
      }
      break;
    case MappingNotify:
      if(ev.xmapping.request != MappingPointer) {
        keys_ungrab();
        XRefreshKeyboardMapping(&ev.xmapping);
        keys_update();
      }
      break;
    case KeyPress:
      if(current && keyaction(ev) == KA_CLOSE)
        delete_window(current);
      if(keyaction(ev) == KA_NEXT || keyaction(ev) == KA_PREV)
        wlist_start(ev);
      if(current && keyaction(ev) == KA_ICONIFY)
        client_iconify(current);
      if(current && keyaction(ev) == KA_MAXIMISE)
        client_maximise(current);
      if(current && keyaction(ev) == KA_FULLSCREEN)
        client_fullscreen(current);
      if(current && keyaction(ev) == KA_EXPAND)
        client_expand(current);
      if(current && keyaction(ev) == KA_TITLE)
        client_toggle_title(current);
      if(current && keyaction(ev) == KA_BOTTOMLEFT) {
        client_move(current, 0, display_height - client_height_total(current));
        client_warp(current);
      }
      if(current && keyaction(ev) == KA_BOTTOMRIGHT) {
        client_move(current, display_width - client_width_total(current), display_height - client_height_total(current));
        client_warp(current);
      }
      if(current && keyaction(ev) == KA_TOPRIGHT) {
        client_move(current, display_width - client_width_total(current), 0);
        client_warp(current);
      }
      if(current && keyaction(ev) == KA_TOPLEFT) {
        client_move(current, 0, 0);
        client_warp(current);
      }
      if(cn && keyaction(ev) == KA_ICONIFY_ALL)
        for(i = 0; i < cn; i++)
          client_iconify(clients[i]);
      if(keyaction(ev) == KA_EXEC)
        spawn(keyarg(ev));
      break;
    case ConfigureNotify:
      if(root == ev.xconfigure.window) {
        display_width = ev.xconfigure.width;
        display_height = ev.xconfigure.height;
        if(evh == wlist_handle_event)
          wlist_update();
        ewmh_update_geometry();
        for(i = 0; i < cn; i++)
          client_update_size(clients[i]);
      }
      break;
    default:
      if(c && ev.type == shape_event)
        set_shape(c);
      break;
  }
}

