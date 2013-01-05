#include "matmenu.h"

int xerrorhandler(Display *display, XErrorEvent *xerror) {
#ifdef DEBUG
  char ret[666];
  XGetErrorText(xerror->display, xerror->error_code, ret, 666);
  printf("x error: %s\n", ret);
#endif
  return 0;
}

