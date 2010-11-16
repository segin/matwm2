#include "matwm.h"

int (*evh)() = NULL;

void handle_event(XEvent ev) {
  int c, i;
  if(evh && evh(ev))
    return;
  for(c = 0; c < cn; c++)
    if(clients[c].parent == ev.xany.window)
      break;
  switch(ev.type) {
    case MapRequest:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xmaprequest.window)
          break;
      if(i < cn) {
        if(clients[c].iconic)
          restore(c);
      } else add_client(ev.xmaprequest.window);
      break;
    case DestroyNotify:
      if(c < cn && clients[c].window == ev.xdestroywindow.window)
        remove_client(c, 2);
      break;
    case UnmapNotify:
      if(c < cn && clients[c].window == ev.xunmap.window)
        remove_client(c, 1);
      break;
    case ConfigureRequest:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xconfigurerequest.window)
          break;
      if(i < cn) {
        resize(i, (ev.xconfigurerequest.value_mask & CWWidth) ? ev.xconfigurerequest.width : clients[i].width, (ev.xconfigurerequest.value_mask & CWHeight) ? ev.xconfigurerequest.height : clients[i].height);
        move(i, (ev.xconfigurerequest.value_mask & CWX) ? ev.xconfigurerequest.x - gxo(i, 0) : clients[i].x, (ev.xconfigurerequest.value_mask & CWY) ? ev.xconfigurerequest.y - gyo(i, 0) : clients[i].y);
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
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xproperty.window)
          break;
     if(i < cn && ev.xproperty.atom == XA_WM_NAME) {
        if(clients[i].name)
          XFree(clients[i].name);
        XFetchName(dpy, clients[i].window, &clients[i].name);
        XClearWindow(dpy, clients[i].parent);
        XClearWindow(dpy, clients[i].icon);
        draw_client(i);
        draw_icon(i);
      }
      if(ev.xproperty.atom == XA_WM_NORMAL_HINTS && i < cn)
        getnormalhints(i);
      if(ev.xproperty.atom == xa_motif_wm_hints && i < cn) {
        get_mwm_hints(i);
        XMoveWindow(dpy, clients[i].window, border(i), border(i) + title(i));
        XResizeWindow(dpy, clients[i].parent, total_width(i), total_height(i));
      }
      break;
    case ClientMessage:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xclient.window)
          break;
      if(i < cn && ev.xclient.message_type == xa_wm_change_state && ev.xclient.data.l[0] == IconicState)
        iconify(i);
      break;
    case EnterNotify:
      if(c == cn)
        for(c = 0; c < cn; c++)
          if(clients[c].window == ev.xcrossing.window)
            break;
      if(c == cn)
        for(c = 0; c < cn; c++)
          if(clients[c].icon == ev.xcrossing.window)
            break;
      if(c < cn)
        focus(c);
      break;
    case Expose:
      for(i = 0; i < cn; i++)
        if(clients[i].icon == ev.xexpose.window)
          break;
      if(ev.xexpose.count == 0) {
        if(c < cn)
          draw_client(c);
        else if(i < cn)
          draw_icon(i);
        else if(ev.xexpose.window == wlist)
          wlist_draw();
      }
      break;
    case ButtonPress:
      if(c < cn && (buttonaction(ev.xbutton.button) == BA_MOVE || buttonaction(ev.xbutton.button) == BA_RESIZE))
        drag_start(ev);
      break;
    case ButtonRelease:
      if(c < cn) {
        if(buttonaction(ev.xbutton.button) == BA_RAISE)
          restack_client(c, 1);
        if(buttonaction(ev.xbutton.button) == BA_LOWER)
          restack_client(c, 0);
      }
      break;
    case MappingNotify:
      if(ev.xmapping.request != MappingPointer) {
        XUngrabKeyboard(dpy, CurrentTime);
        XRefreshKeyboardMapping(&ev.xmapping);
        mapkeys();
      }
      break;
    case KeyPress:
      if(current < cn && iskey(key_close))
        delete_window(current);
      if(iskey(key_next) || iskey(key_prev))
        wlist_start(ev);
      if(current < cn && iskey(key_iconify))
        iconify(current);
      if(current < cn && iskey(key_maximise))
        maximise(current);
      if(current < cn && iskey(key_bottomleft))
        move(current, 0, display_height - total_height(current));
      if(current < cn && iskey(key_bottomright))
        move(current, display_width - total_width(current), display_height - total_height(current));
      if(current < cn && iskey(key_topright))
        move(current, display_width - total_width(current), 0);
      if(current < cn && iskey(key_topleft))
        move(current, 0, 0);
      break;
    case ConfigureNotify:
      if(root == ev.xconfigure.window) {
        display_width = ev.xconfigure.width;
        display_height = ev.xconfigure.height;
      }
      break;
    default:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xany.window)
          break;
      if(i < cn && ev.type == shape_event)
        set_shape(i);
      break;
  }
}

