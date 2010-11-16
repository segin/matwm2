#include "matwm.h"
#include <X11/Xatom.h>

void handle_event(XEvent ev) {
  int c, i;
  for(c = 0; c < cn; c++)
    if(clients[c].parent == ev.xany.window || clients[c].taskbutton == ev.xany.window)
      break;
  switch(ev.type) {
    case MapRequest:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xmaprequest.window)
          break;
      if(i == cn)
        add_client(ev.xmaprequest.window, 1);
      break;
    case UnmapNotify:
      if(c < cn)
        remove_client(c);
      break;
    case ConfigureRequest:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xconfigurerequest.window)
          break;
      configure(i, &ev.xconfigurerequest);
      break;
    case PropertyNotify:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xproperty.window)
          break;
      if(ev.xproperty.atom == XA_WM_NAME && i < cn) {
        if(clients[i].name)
          XFree(clients[i].name);
        XFetchName(dpy, clients[i].window, &clients[i].name);
        XClearWindow(dpy, clients[i].minimised ? clients[i].taskbutton : clients[i].parent);
        clients[i].minimised ? draw_taskbutton(i) : draw_client(i);
      }
      if(ev.xproperty.atom == XA_WM_NORMAL_HINTS && i < cn)
        getnormalhints(i);
      break;
    case EnterNotify:
      if(c < cn)
        focus(c);
      break;
    case Expose:
      if(c < cn && ev.xexpose.count == 0)
        clients[c].minimised ? draw_taskbutton(c) : draw_client(c);
      break;
    case ButtonPress:
      if(c < cn && !clients[c].minimised && (ev.xbutton.button == move_button || ev.xbutton.button == resize_button)) {
          XRaiseWindow(dpy, clients[c].parent);
          drag(c, &ev.xbutton);
        }
      break;
    case ButtonRelease:
      if(ev.xbutton.button == tb_raise_button && ((c < cn && clients[c].minimised) || ev.xbutton.window == taskbar)) {
        XRaiseWindow(dpy, taskbar);
      } else if(ev.xbutton.button == tb_lower_button && ((c < cn && clients[c].minimised) || ev.xbutton.window == taskbar)) {
        XLowerWindow(dpy, taskbar);
      } else if(c < cn)
        if(clients[c].minimised && ev.xbutton.button == tb_restore_button) {
          minimise(c);
        } else if(ev.xbutton.button == raise_button) {
          XRaiseWindow(dpy, clients[c].parent);
        } else if(ev.xbutton.button == lower_button) 
          XLowerWindow(dpy, clients[c].parent);
    case KeyPress:
      if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_s) && current < cn)
        minimise(current);
      if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_Tab))
        next(0, 1);
      if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_a)) {
        next(1, 1);
        XRaiseWindow(dpy, taskbar);
      }
      break;
  }
}

