#include "matwm.h"

Window button_current = None;
int button_down = 0;

void buttons_create(client *c) {
  c->button_parent = XCreateWindow(dpy, c->parent, (c->width + border_width - 1) - button_parent_width, border_width - 1, button_parent_width, text_height, 0,
                                   DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                   CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  c->button_iconify = XCreateWindow(dpy, c->button_parent, 0, 0, text_height, text_height, 0,
                                    DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                    CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  c->button_expand = XCreateWindow(dpy, c->button_parent, text_height + 2, 0, text_height, text_height, 0,
                                   DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                   CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  c->button_maximise = XCreateWindow(dpy, c->button_parent, ((text_height + 2) * 2), 0, text_height, text_height, 0,
                                     DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                     CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  c->button_close = XCreateWindow(dpy, c->button_parent, ((text_height + 2) * 3), 0, text_height, text_height, 0,
                                  DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                  CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XMapWindow(dpy, c->button_parent);
  XMapWindow(dpy, c->button_iconify);
  XMapWindow(dpy, c->button_expand);
  XMapWindow(dpy, c->button_maximise);
  XMapWindow(dpy, c->button_close);
}

void buttons_draw(client *c) {
  button_draw(c, c->button_iconify);
  button_draw(c, c->button_expand);
  button_draw(c, c->button_maximise);
  button_draw(c, c->button_close);
}

void button_draw(client *c, Window b) {
  XClearWindow(dpy, b);
  if(button_current != None)
    XDrawRectangle(dpy, button_current, gc, 0, 0, text_height - 1, text_height - 1);
  if(b == c->button_iconify)
    XDrawRectangle(dpy, c->button_iconify, (c == current) ? gc : igc, 5, 5, text_height - 11, text_height - 11);
  if(b == c->button_expand) {
    XDrawLine(dpy, c->button_expand, (c == current) ? gc : igc, text_height / 2, 3, text_height / 2, text_height - 3);
    XDrawLine(dpy, c->button_expand, (c == current) ? gc : igc, 3, text_height / 2, text_height - 3, text_height / 2);
  }
  if(b == c->button_maximise)
    XDrawRectangle(dpy, c->button_maximise, (c == current) ? gc : igc, 2, 2, text_height - 5, text_height - 5);
  if(b == c->button_close) {
    XDrawLine(dpy, c->button_close, (c == current) ? gc : igc, 2, 2, text_height - 2, text_height - 2);
    XDrawLine(dpy, c->button_close, (c == current) ? gc : igc, 2, text_height - 3, text_height - 2, 1);
  }
}

void buttons_update(client *c) {
  if(c->flags & HAS_BUTTONS && c->width <= button_parent_width + 2) {
    c->flags ^= HAS_BUTTONS;
    XUnmapWindow(dpy, c->button_parent);
  } else if(!(c->flags & HAS_BUTTONS) && c->width > button_parent_width + 2) {
    c->flags |= HAS_BUTTONS;
    XMapWindow(dpy, c->button_parent);
  }
}

int button_handle_event(XEvent ev) {
  int i;
  client *c = NULL;
  for(i = 0; i < cn; i++)
    if(clients[i]->button_iconify == ev.xany.window || clients[i]->button_expand == ev.xany.window || clients[i]->button_maximise == ev.xany.window || clients[i]->button_close == ev.xany.window)
      c = clients[i];
  if(!c)
    return 0;
  switch(ev.type) {
    case Expose:
      button_draw(c, ev.xexpose.window);
      return 1;
    case EnterNotify:
      if(button_down) {
        button_down = 2;
        return 1;
      }
      button_current = ev.xcrossing.window;
      button_draw(c, ev.xcrossing.window);
      return 1;
    case LeaveNotify:
      if(button_down == 2)
        button_down = 1;
      button_current = None;
      button_draw(c, ev.xcrossing.window);
      return 1;
    case ButtonPress:
      if(ev.xbutton.button == Button1)
        button_down = 1;
      return 1;
    case ButtonRelease:
      if(ev.xbutton.button == Button1) {
        if(button_current == ev.xbutton.window) {
          if(ev.xbutton.window == c->button_iconify)
            client_iconify(c);
          if(ev.xbutton.window == c->button_expand)
            client_expand(c);
          if(ev.xbutton.window == c->button_maximise)
            client_maximise(c);
          if(ev.xbutton.window == c->button_close)
            delete_window(c);
        }
        if(button_down == 2) {
          button_current = ev.xbutton.window;
          button_draw(c, ev.xbutton.window);
        }
        button_down = 0;
      }
      return 1;
  }
  return 0;
}

