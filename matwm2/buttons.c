#include "matwm.h"

Window button_current, button_down;

void buttons_create(client *c) {
  c->button_parent = XCreateWindow(dpy, c->parent, (c->width + border_width) - button_parent_width, border_width, button_parent_width, text_height, 0,
                                   DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                   CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  c->button_iconify = XCreateWindow(dpy, c->button_parent, 2, 0, text_height, text_height, 0,
                                    DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                    CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  c->button_expand = XCreateWindow(dpy, c->button_parent, text_height + 4, 0, text_height, text_height, 0,
                                   DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                   CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  c->button_maximise = XCreateWindow(dpy, c->button_parent, 2 + ((text_height + 2) * 2), 0, text_height, text_height, 0,
                                     DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                     CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  c->button_close = XCreateWindow(dpy, c->button_parent, 2 + ((text_height + 2) * 3), 0, text_height, text_height, 0,
                                  DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                  CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XMapWindow(dpy, c->button_parent);
  XMapWindow(dpy, c->button_iconify);
  XMapWindow(dpy, c->button_expand);
  XMapWindow(dpy, c->button_maximise);
  XMapWindow(dpy, c->button_close);
}

void buttons_draw(client *c) {
  XClearWindow(dpy, c->button_parent);
  XClearWindow(dpy, c->button_iconify);
  XClearWindow(dpy, c->button_expand);
  XClearWindow(dpy, c->button_maximise);
  XClearWindow(dpy, c->button_close);
  if(button_current != root)
    XDrawRectangle(dpy, button_current, gc, 0, 0, text_height - 1, text_height - 1);
  XDrawRectangle(dpy, c->button_iconify, (c == current) ? gc : igc, 5, 5, text_height - 11, text_height - 11);
  XDrawLine(dpy, c->button_expand, (c == current) ? gc : igc, text_height / 2, 3, text_height / 2, text_height - 3);
  XDrawLine(dpy, c->button_expand, (c == current) ? gc : igc, 3, text_height / 2, text_height - 3, text_height / 2);
  XDrawRectangle(dpy, c->button_maximise, (c == current) ? gc : igc, 2, 2, text_height - 5, text_height - 5);
  XDrawLine(dpy, c->button_close, (c == current) ? gc : igc, 2, 2, text_height - 2, text_height - 2);
  XDrawLine(dpy, c->button_close, (c == current) ? gc : igc, 2, text_height - 3, text_height - 2, 1);
}

int handle_button_event(XEvent ev) {
  int i;
  client *c = NULL;
  for(i = 0; i < cn; i++)
    if(clients[i]->button_iconify == ev.xany.window || clients[i]->button_expand == ev.xany.window || clients[i]->button_maximise == ev.xany.window || clients[i]->button_close == ev.xany.window)
      c = clients[i];
  if(!c)
    return 0;
  switch(ev.type) {
    case Expose:
      buttons_draw(c);
      return 1;
    case EnterNotify:
      button_current = ev.xcrossing.window;
      buttons_draw(c);
      return 1;
    case LeaveNotify:
      button_current = root; // make sure its not a button (i chose root because its always there and i assumed any value could be a window)
      button_down = root;
      buttons_draw(c);
      return 1;
    case ButtonPress:
      if(ev.xbutton.button == Button1)
        button_down = ev.xbutton.window;
      return 1;
    case ButtonRelease:
      if(ev.xbutton.button == Button1 && button_down == ev.xbutton.window) {
        if(ev.xbutton.window == c->button_iconify)
          iconify(c);
        if(ev.xbutton.window == c->button_expand)
          expand(c);
        if(ev.xbutton.window == c->button_maximise)
          maximise(c);
        if(ev.xbutton.window == c->button_close)
          delete_window(c);
      }
      return 1;
  }
  return 0;
}

