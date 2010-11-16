#include "matwm.h"

Atom ewmh_atoms[EWMH_ATOM_COUNT];
long ewmh_strut[4];

void ewmh_initialize(void) {
  long cd = 0;
  long vd_count = 1;
  long ds[] = {display_width, display_height};
  long vp[] = {0, 0};
  ewmh_atoms[NET_SUPPORTED] = XInternAtom(dpy, "_NET_SUPPORTED", False);
  ewmh_atoms[NET_SUPPORTING_WM_CHECK] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
  ewmh_atoms[NET_WM_NAME] =  XInternAtom(dpy, "_NET_WM_NAME", False);
  ewmh_atoms[NET_NUMBER_OF_DESKTOPS] = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
  ewmh_atoms[NET_DESKTOP_GEOMETRY] = XInternAtom(dpy, "_NET_DESKTOP_GEOMETRY", False);
  ewmh_atoms[NET_DESKTOP_VIEWPORT] = XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", False);
  ewmh_atoms[NET_WORKAREA] = XInternAtom(dpy, "_NET_WORKAREA", False);
  ewmh_atoms[NET_WM_STRUT] = XInternAtom(dpy, "_NET_WM_STRUT", False);
  ewmh_atoms[NET_WM_STRUT_PARTIAL] = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
  ewmh_atoms[NET_CURRENT_DESKTOP] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
  ewmh_atoms[NET_DESKTOP_NAMES] = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
  ewmh_atoms[NET_CLIENT_LIST] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
  ewmh_atoms[NET_CLIENT_LIST_STACKING] = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
  ewmh_atoms[NET_ACTIVE_WINDOW] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
  ewmh_atoms[NET_WM_STATE] = XInternAtom(dpy, "_NET_WM_STATE", False);
  ewmh_atoms[NET_CLOSE_WINDOW] = XInternAtom(dpy, "_NET_CLOSE_WINDOW", False);
  ewmh_atoms[NET_WM_WINDOW_TYPE] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
  ewmh_atoms[NET_WM_WINDOW_TYPE_DESKTOP] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
  ewmh_atoms[NET_WM_WINDOW_TYPE_DOCK] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  XChangeProperty(dpy, root, ewmh_atoms[NET_SUPPORTED], XA_ATOM, 32, PropModeReplace, (unsigned char *) &ewmh_atoms, EWMH_ATOM_COUNT);
  XChangeProperty(dpy, root, ewmh_atoms[NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wlist, 1);
  XChangeProperty(dpy, wlist, ewmh_atoms[NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wlist, 1);
  XChangeProperty(dpy, wlist, ewmh_atoms[NET_WM_NAME], XA_STRING, 8, PropModeReplace, (unsigned char *) NAME, strlen(NAME));
  XChangeProperty(dpy, root, ewmh_atoms[NET_NUMBER_OF_DESKTOPS], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &vd_count, 1);
  XChangeProperty(dpy, root, ewmh_atoms[NET_DESKTOP_GEOMETRY], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &ds, 2);
  XChangeProperty(dpy, root, ewmh_atoms[NET_DESKTOP_VIEWPORT], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &vp, 2);
  XChangeProperty(dpy, root, ewmh_atoms[NET_CURRENT_DESKTOP], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &cd, 1);
  XDeleteProperty(dpy, root, ewmh_atoms[NET_DESKTOP_NAMES]);
  ewmh_update_clist();
}

int ewmh_handle_event(XEvent ev) {
  client *c;
  int i;
  switch(ev.type) {
    case ClientMessage:
      c = owner(ev.xclient.window);
      if(ev.xclient.message_type == ewmh_atoms[NET_CLOSE_WINDOW]) {
        if(c)
          delete_window(c);
        return 1;
      }
      if(ev.xclient.message_type == ewmh_atoms[NET_ACTIVE_WINDOW]) {
        if(c) {
          if(c->flags & ICONIC)
            client_restore(c);
          else if(client_number(stacking, c) == 0 || c->layer > stacking[client_number(stacking, c) - 1]->layer)
            client_iconify(c);
          else
            client_raise(c);
        }
        return 1;
      }
      if(ev.xclient.message_type == ewmh_atoms[NET_WM_STATE]) {
        printf("change state for %s to %s\n", c ? c->name : "unknown window", XGetAtomName(dpy, ev.xclient.data.l[0]));
        return 1;
      }
      break;
    case PropertyNotify:
      if(ev.xproperty.atom == ewmh_atoms[NET_WM_STRUT_PARTIAL] || ev.xproperty.atom == ewmh_atoms[NET_WM_STRUT]) {
        ewmh_update_strut();
        return 1;
      }
      break;
    }
  return 0;
}

int get_ewmh_hints(client *c) {
  Atom rt, *data;
  int i, rf;
  unsigned long nir, bar;
  if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_WINDOW_TYPE], 0, 1, False, XA_ATOM, &rt, &rf, &nir, &bar, (unsigned char **) &data) == Success) {
    if(nir) {
      if(*data == ewmh_atoms[NET_WM_WINDOW_TYPE_DESKTOP]) {
        c->flags ^= c->flags & (CAN_MOVE | CAN_RESIZE | HAS_BORDER | HAS_TITLE);
        c->flags |= NO_STRUT;
        c->layer = DESKTOP;
      }
      if(*data == ewmh_atoms[NET_WM_WINDOW_TYPE_DOCK]) {
        c->flags ^= c->flags & (CAN_MOVE | CAN_RESIZE | HAS_BORDER | HAS_TITLE);
        c->flags |= NO_STRUT;
      }
    }
    XFree((void *) data);
  }
}

void ewmh_set_active(client *c) {
  XChangeProperty(dpy, root, ewmh_atoms[NET_ACTIVE_WINDOW], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &c->window, 1);
}

void ewmh_update_stacking(void) {
  int i;
  Window data[cn];
  for(i = 0; i < cn; i++)
    data[cn - (i + 1)] = stacking[i]->window;
  XChangeProperty(dpy, root, ewmh_atoms[NET_CLIENT_LIST_STACKING], XA_WINDOW, 32, PropModeReplace, (unsigned char *) data, cn);
}

void ewmh_update_clist(void) {
  int i;
  Window data[cn];
  for(i = 0; i < cn; i++)
    data[cn - (i + 1)] = clients[i]->window;
  XChangeProperty(dpy, root, ewmh_atoms[NET_CLIENT_LIST], XA_WINDOW, 32, PropModeReplace, (unsigned char *) data, cn);
  ewmh_update_stacking();
  ewmh_update_strut();
}

void ewmh_update_strut(void) {
  Atom rt;
  int i, rf;
  unsigned long nir, bar;
  long workarea[4], *data;
  for(i = 0; i < 4; i++)
    ewmh_strut[i] = 0;
  for(i = 0; i < cn; i++) {
    if(XGetWindowProperty(dpy, clients[i]->window, ewmh_atoms[NET_WM_STRUT_PARTIAL], 0, 4, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &data) != Success)
      if(XGetWindowProperty(dpy, clients[i]->window, ewmh_atoms[NET_WM_STRUT], 0, 4, False, XA_CARDINAL, &rt, &rf, &nir, &bar, (unsigned char **) &data) != Success)
        continue;
    if(nir == 4) {
        ewmh_strut[0] += data[0];
        ewmh_strut[1] += data[1];
        ewmh_strut[2] += data[2];
        ewmh_strut[3] += data[3];
    }
    XFree((void *) data);
  }
  workarea[0] = ewmh_strut[0];
  workarea[1] = ewmh_strut[2];
  workarea[2] = display_width - (ewmh_strut[0] + ewmh_strut[1]);
  workarea[3] = display_height - (ewmh_strut[2] + ewmh_strut[3]);
  XChangeProperty(dpy, root, ewmh_atoms[NET_WORKAREA], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &workarea, 4);
}

