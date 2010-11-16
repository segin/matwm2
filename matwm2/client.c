#include "matwm.h"
#include <string.h>

client *clients;
int cn = 0;

void new_client(Window w) {
  XWindowAttributes attr;
  XSetWindowAttributes p_attr;
  long sr;
  clients = (client *) realloc((client *) clients, (cn + 1) * sizeof(client));
  XSelectInput(dpy, w, PropertyChangeMask);
  XSetWindowBorderWidth(dpy, w, 0);
  XGetWindowAttributes(dpy, w, &attr);
  clients[cn].x = attr.x;
  clients[cn].y = attr.y;
  clients[cn].width = attr.width;
  clients[cn].height = attr.height;
  clients[cn].window = w;
  XGetWMNormalHints(dpy, clients[cn].window, &clients[cn].normal_hints, &sr);
  XFetchName(dpy, clients[cn].window, &clients[cn].name);
  p_attr.override_redirect = True;
  p_attr.background_pixel = ibg.pixel;
  p_attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | EnterWindowMask | LeaveWindowMask;
  clients[cn].parent = XCreateWindow(dpy, root, clients[cn].x, clients[cn].y, clients[cn].width + (border_width * 2), clients[cn].height + (border_width * 2) + title_height, 0,
                                     DefaultDepth(dpy, screen), CopyFromParent, DefaultVisual(dpy, screen),
                                     CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
  XReparentWindow(dpy, clients[cn].window, clients[cn].parent, border_width, border_width + title_height);
  XMapWindow(dpy, clients[cn].window);
  XMapRaised(dpy, clients[cn].parent);
  grab_button(clients[cn].parent, AnyButton, mousemodmask, ButtonPressMask | ButtonReleaseMask);
  cn++;
}

void remove_client(int n) {
  int i;
  XDestroyWindow(dpy, clients[n].parent);
  XFree(clients[n].name);
  cn--;
  for(i = n; i < cn; i++)
    clients[i] = clients[i + 1];
  clients = (client *) realloc((client *) clients, (cn + 1) * sizeof(client));
}

void client_draw(int n) {
  XDrawString(dpy, clients[n].parent, gc, border_width + font->max_bounds.lbearing, border_width + font->max_bounds.ascent, clients[n].name, strlen(clients[n].name));
}

void add_initial_clients(void) {
  unsigned int i, nwins;
  Window dw1, dw2, *wins;
  XWindowAttributes winattr;
  XSetWindowAttributes attr;
  XQueryTree(dpy, root, &dw1, &dw2, &wins, &nwins);
  for(i = 0; i < nwins; i++) {
    XGetWindowAttributes(dpy, wins[i], &winattr);
    if(!winattr.override_redirect && winattr.map_state == IsViewable)
      new_client(wins[i]);
  }
  XFree(wins);
}

int has_protocol(Window w, Atom protocol) {
  int i, count, ret = 0;
  Atom *protocols;
  if(XGetWMProtocols(dpy, w, &protocols, &count)) {
    for(i = 0; i < count; i++)
      if(protocols[i] == protocol)
        ret++;
    XFree(protocols);
  }
  return ret;
}

