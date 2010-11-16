#include "matwm.h"
#include <stdio.h>

int xerrorhandler(Display *display, XErrorEvent *xerror) {
  char ret[50];
  if (xerror->error_code == BadAccess && xerror->resourceid == root) {
    fprintf(stderr,"Another window manager is running on display %s.\n", XDisplayName(0));
    exit(1);
  }
  XGetErrorText(xerror->display, xerror->error_code, ret, 50);
  printf("%s\n", ret);
  return 0;
}

