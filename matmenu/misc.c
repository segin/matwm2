#include "matmenu.h"

void spawn(char *cmd) {
  if(vfork() == 0) {
    setsid();
    if(dn)
      setenv("DISPLAY", dn, 1);
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
      if(r <= 0) {
        free((void *) *buf);
      } else (*buf)[r] = 0;
    }
    close(fd);
  }
  return r;
}

void readcomp(int fd) {
  unsigned int p = 0, nl = CLINES_PREALLOC, cll = STR_PREALLOC;
  comp = (char **) malloc(nl * sizeof(char *));
  comp[ncomp] = (char *) malloc(cll);
  while(1) {
    if(!read(fd, comp[ncomp] + p, 1) || comp[ncomp][p] == EOF)
      break;
    if(comp[ncomp][p] == '\n') {
      comp[ncomp][p] = 0;
      ncomp++;
      p = 0;
      if(ncomp >= nl) {
        nl += CLINES_PREALLOC;
        comp = (char **) realloc((void *) comp, nl * sizeof(char *));
      }
      cll = STR_PREALLOC;
      comp[ncomp] = (char *) malloc(cll);
      continue;
    }
    p++;
    if(p >= cll) {
      cll += STR_PREALLOC;
      comp[ncomp] = realloc(comp[ncomp], cll);
    }
  }
  comp[ncomp][p] = 0;
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

