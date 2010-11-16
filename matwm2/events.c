#include "matwm.h"
#include <X11/Xatom.h>

void handle_event(XEvent ev) {
  int c, i;
  long sr;
  for(c = 0; c < cn; c++)
     if(clients[c].parent == ev.xany.window)
       break;
  switch(ev.type) {
    case MapRequest:
      new_client(ev.xmaprequest.window);
      break;
    case UnmapNotify:
        if(c < cn)
          remove_client(c);
      break;
    case ConfigureRequest:
      if(c < cn) {
        move(c, ev.xconfigurerequest.value_mask & CWX ? ev.xconfigurerequest.x : clients[c].x, ev.xconfigurerequest.value_mask & CWY ? ev.xconfigurerequest.y : clients[c].y);
        resize(c, ev.xconfigurerequest.width, ev.xconfigurerequest.height);
      } else XMoveResizeWindow(dpy, ev.xconfigure.window, ev.xconfigure.x, ev.xconfigure.y, ev.xconfigure.width, ev.xconfigure.height);
      break;
    case PropertyNotify:
      if(ev.xproperty.atom == XA_WM_NAME)
        for(i = 0; i < cn; i++)
          if(clients[i].window == ev.xproperty.window) {
            XFree(clients[i].name);
            XFetchName(dpy, clients[i].window, &clients[i].name);
            XClearWindow(dpy, clients[i].parent);
            client_draw(i);
          }
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
      if(c < cn)
        buttonpress(c, ev.xbutton.button);
      break;
  }
}

