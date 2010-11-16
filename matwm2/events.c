#include "matwm.h"

void handle_event(XEvent ev) {
  int c, i;
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
      } else add_client(ev.xmaprequest.window, 1);
      break;
    case DestroyNotify:
      if(c < cn)
        remove_client(c, 0);
      break;
    case UnmapNotify:
      if(c < cn)
        if(clients[c].iconic != 1) {
          remove_client(c, 1);
        } else if(clients[c].iconic)
          clients[c].iconic = 2;
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
        clients[i].iconic ? draw_icon(i) : draw_client(i);
      }
      if(ev.xproperty.atom == XA_WM_NORMAL_HINTS && i < cn)
        getnormalhints(i);
      break;
    case ClientMessage:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xclient.window)
          break;
      if(i < cn && ev.xclient.message_type == xa_wm_change_state && ev.xclient.data.l[0] == IconicState && !clients[i].iconic)
        restore(i);
      break;
    case EnterNotify:
      if(c < cn)
        focus(c);
      break;
    case Expose:
      if(c < cn && ev.xexpose.count == 0)
        clients[c].iconic ? draw_icon(c) : draw_client(c);
      break;
    case ButtonPress:
      if(c < cn && !clients[c].iconic && (ev.xbutton.button == move_button || ev.xbutton.button == resize_button)) {
          restack_client(c, 1);
          drag(c, &ev.xbutton);
        }
      break;
    case ButtonRelease:
      if(c < cn && clients[c].iconic && ev.xbutton.button == icon_raise_button) {
        restack_icons(1);
      } else if(c < cn && clients[c].iconic && ev.xbutton.button == icon_lower_button) {
        restack_icons(0);
      } else if(c < cn)
        if(clients[c].iconic && ev.xbutton.button == icon_restore_button) {
          restore(c);
        } else if(ev.xbutton.button == raise_button) {
          restack_client(c, 1);
        } else if(ev.xbutton.button == lower_button) 
          restack_client(c, 0);
      break;
    case KeyPress:
      if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_q) && current < cn)
        delete_window(current);
      if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_Tab))
        next(0, 1);
      if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_a)) {
        next(1, 1);
        restack_icons(1);
      }
      if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_s) && current < cn) {
        icons_ontop = 0;
        clients[current].iconic ? restore(current) : iconify(current);
        if(!clients[current].iconic)
          XWarpPointer(dpy, None, clients[current].parent, 0, 0, 0, 0, clients[current].width + border_width,  clients[current].height + border_width + title_height);
      }
      break;
  }
}

