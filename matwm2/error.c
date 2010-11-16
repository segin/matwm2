#include "matwm.h"
#include <stdio.h>

int xerrorhandler(Display *display, XErrorEvent *xerror) {
#ifdef DEBUG
  char ret[666];
  XGetErrorText(xerror->display, xerror->error_code, ret, 666);
  printf("x error: %s\n", ret);
#endif
  if(xerror->error_code == BadAccess && xerror->resourceid == root) {
    fprintf(stderr,"error: root window at display %s is not available\n", XDisplayName(0));
    exit(1);
  }
  return 0;
}

