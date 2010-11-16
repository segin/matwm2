#include "matwm.h"

int (*evh)() = NULL;

void handle_event(XEvent ev) {
  client *c = owner(ev.xany.window);
//  if(c) printf("%s: %s\n", c->name, event_name(ev));
  if(evh && evh(ev))
    return;
  switch(ev.type) {
    case MapRequest:
      if(c) {
        if(c->iconic)
          restore(c);
      } else add_client(ev.xmaprequest.window);
      break;
    case DestroyNotify:
      if(c && c->window == ev.xdestroywindow.window)
        remove_client(c, 2);
        break;
    case UnmapNotify:
      if(c && c->window == ev.xunmap.window)
        remove_client(c, 1);
        break;
    case ConfigureRequest:
      if(c) {
        resize(c, (ev.xconfigurerequest.value_mask & CWWidth) ? ev.xconfigurerequest.width : c->width, (ev.xconfigurerequest.value_mask & CWHeight) ? ev.xconfigurerequest.height : c->height);
        move(c, (ev.xconfigurerequest.value_mask & CWX) ? ev.xconfigurerequest.x - gxo(c, 0) : c->x, (ev.xconfigurerequest.value_mask & CWY) ? ev.xconfigurerequest.y - gyo(c, 0) : c->y);
      } else {
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
        if(c->name != NO_TITLE)
          XFree(c->name);
        XFetchName(dpy, c->window, &c->name);
        XClearWindow(dpy, c->parent);
        XClearWindow(dpy, c->icon);
        draw_client(c);
        draw_icon(c);
      }
      if(ev.xproperty.atom == XA_WM_NORMAL_HINTS && c)
        getnormalhints(c);
      if(ev.xproperty.atom == xa_motif_wm_hints && c) {
        get_mwm_hints(c);
        XMoveWindow(dpy, c->window, border(c), border(c) + title(c));
        XResizeWindow(dpy, c->parent, total_width(c), total_height(c));
      }
      break;
    case ClientMessage:
      if(c && ev.xclient.message_type == xa_wm_change_state && ev.xclient.data.l[0] == IconicState)
        iconify(c);
      break;
    case EnterNotify:
      if(c)
        focus(c);
      break;
    case Expose:
      if(ev.xexpose.count == 0) {
        if(c && ev.xexpose.window == c->parent)
          draw_client(c);
        if(evh == wlist_handle_event && c && ev.xexpose.window == c->icon)
          draw_icon(c);
      }
      break;
    case ButtonPress:
      if(c && (buttonaction(ev.xbutton.button) == BA_MOVE || buttonaction(ev.xbutton.button) == BA_RESIZE))
        drag_start(ev);
      break;
    case ButtonRelease:
      if(c) {
        if(buttonaction(ev.xbutton.button) == BA_RAISE)
          raise_client(c);
        if(buttonaction(ev.xbutton.button) == BA_LOWER)
          lower_client(c);
      }
      break;
    case MappingNotify:
      if(ev.xmapping.request != MappingPointer) {
        XUngrabKeyboard(dpy, CurrentTime);
        XRefreshKeyboardMapping(&ev.xmapping);
      }
      break;
    case KeyPress:
      if(current && keyaction(ev) == KA_CLOSE)
        delete_window(current);
      if(keyaction(ev) == KA_NEXT || keyaction(ev) == KA_PREV)
        wlist_start(ev);
      if(current && keyaction(ev) == KA_ICONIFY)
        iconify(current);
      if(current && keyaction(ev) == KA_MAXIMISE)
        maximise(current);
      if(current && keyaction(ev) == KA_BOTTOMLEFT)
        move(current, 0, display_height - total_height(current));
      if(current && keyaction(ev) == KA_BOTTOMRIGHT)
        move(current, display_width - total_width(current), display_height - total_height(current));
      if(current && keyaction(ev) == KA_TOPRIGHT)
        move(current, display_width - total_width(current), 0);
      if(current && keyaction(ev) == KA_TOPLEFT)
        move(current, 0, 0);
      if(keyaction(ev) == KA_EXEC)
        popen(evkey(ev)->arg, "r");
      break;
    case ConfigureNotify:
      if(root == ev.xconfigure.window) {
        display_width = ev.xconfigure.width;
        display_height = ev.xconfigure.height;
        if(evh == wlist_handle_event)
          wlist_update();
      }
      break;
    default:
      if(c && ev.type == shape_event)
        set_shape(c);
      break;
  }
}

