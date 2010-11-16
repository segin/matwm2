#include "matwm.h"
#include <X11/Xatom.h>

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
        XClearWindow(dpy, clients[i].parent);
        client_draw(i);
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
        client_draw(c);
      break;
    case ButtonPress:
      if(c < cn) {
        if(ev.xbutton.button == move_button || ev.xbutton.button == resize_button) {
          XRaiseWindow(dpy, clients[c].parent);
          drag(c, &ev.xbutton);
        } else if(ev.xbutton.button == lower_button)
          XLowerWindow(dpy, clients[c].parent);
      }
      break;
    case KeyPress:
      if(ev.xkey.keycode == XKeysymToKeycode(dpy, XK_Tab))
        next();
      break;
  }
}

