#include "matwm.h"

Atom ewmh_atoms[EWMH_ATOM_COUNT];
long ewmh_strut[4];

void ewmh_initialize(void) {
  long cd = 0;
  long vd_count = 1;
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
  ewmh_atoms[NET_WM_DESKTOP] = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
  ewmh_atoms[NET_CLIENT_LIST] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
  ewmh_atoms[NET_CLIENT_LIST_STACKING] = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
  ewmh_atoms[NET_ACTIVE_WINDOW] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
  ewmh_atoms[NET_WM_STATE] = XInternAtom(dpy, "_NET_WM_STATE", False);
  ewmh_atoms[NET_WM_STATE_FULLSCREEN] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
  ewmh_atoms[NET_WM_STATE_MAXIMIZED_HORZ] = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
  ewmh_atoms[NET_WM_STATE_MAXIMIZED_VERT] = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
  ewmh_atoms[NET_CLOSE_WINDOW] = XInternAtom(dpy, "_NET_CLOSE_WINDOW", False);
  ewmh_atoms[NET_WM_WINDOW_TYPE] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
  ewmh_atoms[NET_WM_WINDOW_TYPE_DESKTOP] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
  ewmh_atoms[NET_WM_WINDOW_TYPE_DOCK] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  ewmh_atoms[NET_WM_ALLOWED_ACTIONS] = XInternAtom(dpy, "_NET_WM_ALLOWED_ACTIONS", False);
  ewmh_atoms[NET_WM_ACTION_MINIMIZE] = XInternAtom(dpy, "_NET_WM_ACTION_MINIMIZE", False);
  ewmh_atoms[NET_WM_ACTION_CLOSE] = XInternAtom(dpy, "_NET_WM_ACTION_CLOSE", False);
  ewmh_atoms[NET_WM_ACTION_MAXIMIZE_HORZ] = XInternAtom(dpy, "_NET_WM_ACTION_MAXIMIZE_HORZ", False);
  ewmh_atoms[NET_WM_ACTION_MAXIMIZE_VERT] = XInternAtom(dpy, "_NET_WM_ACTION_MAXIMIZE_VERT", False);
  ewmh_atoms[NET_WM_ACTION_FULLSCREEN] = XInternAtom(dpy, "_NET_WM_ACTION_FULLSCREEN", False);
  ewmh_atoms[NET_WM_ACTION_MOVE] = XInternAtom(dpy, "_NET_WM_ACTION_MOVE", False);
  ewmh_atoms[NET_WM_ACTION_RESIZE] = XInternAtom(dpy, "_NET_WM_ACTION_RESIZE", False);
  ewmh_atoms[NET_WM_MOVERESIZE] = XInternAtom(dpy, "_NET_WM_MOVERESIZE", False);
  XChangeProperty(dpy, root, ewmh_atoms[NET_SUPPORTED], XA_ATOM, 32, PropModeReplace, (unsigned char *) &ewmh_atoms, EWMH_ATOM_COUNT);
  XChangeProperty(dpy, root, ewmh_atoms[NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wlist, 1);
  XChangeProperty(dpy, wlist, ewmh_atoms[NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &wlist, 1);
  XChangeProperty(dpy, wlist, ewmh_atoms[NET_WM_NAME], XA_STRING, 8, PropModeReplace, (unsigned char *) NAME, strlen(NAME));
  XChangeProperty(dpy, root, ewmh_atoms[NET_DESKTOP_VIEWPORT], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &vp, 2);
  XChangeProperty(dpy, root, ewmh_atoms[NET_NUMBER_OF_DESKTOPS], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &vd_count, 1);
  XChangeProperty(dpy, root, ewmh_atoms[NET_CURRENT_DESKTOP], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &cd, 1);
  XDeleteProperty(dpy, root, ewmh_atoms[NET_DESKTOP_NAMES]);
  ewmh_update_geometry();
}

int ewmh_handle_event(XEvent ev) {
  client *c;
  int i;
  switch(ev.type) {
    case ClientMessage:
      c = owner(ev.xclient.window);
      if(c && ev.xclient.message_type == ewmh_atoms[NET_WM_MOVERESIZE]) {
        if(ev.xclient.data.l[2] == NET_WM_MOVERESIZE_MOVE || ev.xclient.data.l[2] == NET_WM_MOVERESIZE_MOVE_KEYBOARD) {
          client_focus(c);
          int xo = client_width_total_intern(c) / 2, yo = client_height_total_intern(c) / 2;
          XWarpPointer(dpy, None, c->parent, 0, 0, 0, 0, xo, yo);
          drag_start(MOVE, AnyButton, client_x(c) + xo, client_y(c) + yo);
        }
        if(ev.xclient.data.l[2] == NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT || ev.xclient.data.l[2] == NET_WM_MOVERESIZE_SIZE_KEYBOARD) {
          client_focus(c);
          drag_start(RESIZE, AnyButton, ev.xclient.data.l[0], ev.xclient.data.l[1]);
        }
      }
      if(ev.xclient.message_type == ewmh_atoms[NET_CLOSE_WINDOW]) {
        if(c)
          delete_window(c);
        return 1;
      }
      if(ev.xclient.message_type == ewmh_atoms[NET_ACTIVE_WINDOW]) {
        if(c) {
          if(c->flags & ICONIC) {
            client_restore(c);
            client_focus(c);
          } else {
            client_raise(c);
            client_focus(c);
          }
        }
        return 1;
      }
      if(ev.xclient.message_type == ewmh_atoms[NET_WM_STATE]) {
        if(c && ev.xclient.data.l[1] == ewmh_atoms[NET_WM_STATE_MAXIMIZED_HORZ] || ev.xclient.data.l[1] == ewmh_atoms[NET_WM_STATE_MAXIMIZED_VERT])
          client_maximise(c);
        if(c && ev.xclient.data.l[1] == ewmh_atoms[NET_WM_STATE_FULLSCREEN] && (ev.xclient.data.l[0] == NET_WM_STATE_TOGGLE || (ev.xclient.data.l[0] == NET_WM_STATE_ADD && !(c->flags & FULLSCREEN)) || (ev.xclient.data.l[0] == NET_WM_STATE_REMOVE && (c->flags & FULLSCREEN))))
          client_fullscreen(c);
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
        c->flags |= NO_STRUT | DONT_LIST | FULLSCREEN;
        c->layer = DESKTOP;
      }
      if(*data == ewmh_atoms[NET_WM_WINDOW_TYPE_DOCK]) {
        c->flags ^= c->flags & (CAN_MOVE | CAN_RESIZE | HAS_BORDER | HAS_TITLE);
        c->flags |= NO_STRUT | DONT_LIST;
      }
    }
    XFree((void *) data);
  }
  if(XGetWindowProperty(dpy, c->window, ewmh_atoms[NET_WM_STATE], 0, 1, False, XA_ATOM, &rt, &rf, &nir, &bar, (unsigned char **) &data) == Success) {
    if(nir) {
      if(*data == ewmh_atoms[NET_WM_STATE_FULLSCREEN]) {
        c->flags |= FULLSCREEN;
      }
    }
    XFree((void *) data);
  }
}

void ewmh_update_geometry(void) {
  long ds[] = {display_width, display_height};
  XChangeProperty(dpy, root, ewmh_atoms[NET_DESKTOP_GEOMETRY], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &ds, 2);
}

void ewmh_set_desktop(client *c, long desktop) {
  XChangeProperty(dpy, c->window, ewmh_atoms[NET_WM_DESKTOP], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &desktop, 1);
}

void ewmh_set_active(client *c) {
  XChangeProperty(dpy, root, ewmh_atoms[NET_ACTIVE_WINDOW], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &c->window, 1);
}

void ewmh_update_allowed_actions(client *c) {
  int nactions = 2;
  Atom actions[12] = {ewmh_atoms[NET_WM_ACTION_MINIMIZE], ewmh_atoms[NET_WM_ACTION_CLOSE]};
  if(c->flags & CAN_MOVE && c->flags & CAN_RESIZE) {
    actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MAXIMIZE_HORZ];
    actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MAXIMIZE_VERT];
    actions[nactions++] = ewmh_atoms[NET_WM_ACTION_FULLSCREEN];
  }
  if(c->flags & CAN_MOVE)
    actions[nactions++] = ewmh_atoms[NET_WM_ACTION_MOVE];
  if(c->flags & CAN_RESIZE)
    actions[nactions++] = ewmh_atoms[NET_WM_ACTION_RESIZE];
  XChangeProperty(dpy, c->window, ewmh_atoms[NET_WM_ALLOWED_ACTIONS], XA_ATOM, 32, PropModeReplace, (unsigned char *) &actions, nactions);
}

void ewmh_update_state(client *c) {
  int statec = 0;
  Atom state[12];
  if(c->flags & MAXIMISED) {
    state[statec++] = ewmh_atoms[NET_WM_STATE_MAXIMIZED_HORZ];
    state[statec++] = ewmh_atoms[NET_WM_STATE_MAXIMIZED_VERT];
  }
  if(c->flags & FULLSCREEN)
    state[statec++] = ewmh_atoms[NET_WM_STATE_FULLSCREEN];
  XChangeProperty(dpy, c->window, ewmh_atoms[NET_WM_STATE], XA_ATOM, 32, PropModeReplace, (unsigned char *) &state, statec);
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

