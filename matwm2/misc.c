#include "matwm.h"

void spawn(char *cmd) {
  if(vfork() == 0) {
    setsid();
    execlp("sh", "sh", "-c", cmd, (char *) 0);
    _exit(1);
  }
}

int read_file(char *path, char **buf) {
  struct stat sb;
  int r = 0, fd = open(path, O_RDONLY);
  if(fd > 0) {
    if(fstat(fd, &sb) == 0) {
      *buf = (char *) malloc(sb.st_size);
      if(buf == NULL)
        error();
      r = read(fd, (void *) *buf, sb.st_size);
      if(r <= 0)
        free((void *) *buf);
      else buf[r] = 0;
    }
    close(fd);
  }
  return r;
}

char *eat(char **str, char *until) {
  char *ret = NULL, *c;
  while(1) {
    if(!ret && (**str != ' ' && **str != '\t'))
      ret = *str;
    if(**str == 0) {
      *str = NULL;
      return ret;
    }
    c = until;
    while(*c) {
      if(ret && (**str == *c || **str == 0)) {
        **str = 0;
        (*str)++;
        return ret;
      }
      c++;
    }
    (*str)++;
  }
}

