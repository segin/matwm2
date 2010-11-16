#include "matwm.h"

void spawn(char *cmd) {
	if(rfork(RFPROC | RFNOWAIT) == 0) {
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
			*buf = (char *) malloc(sb.st_size + 1);
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

char *eat(char **str, char *until) {
	char *ret = NULL, *c;
	int literal = 0;
	while(1) {
		if(!ret && (**str != ' ' && **str != '\t'))
			ret = *str;
		if(**str == '\\' && !literal) {
			literal = 1;
			(*str)++;
			continue;
		}
		if(**str == 0) {
			*str = NULL;
			return ret;
		}
		if(literal)
			literal = 0;
		else {
			c = until;
			while(ret && *c) {
  			if(**str == *c) {
					**str = 0;
					(*str)++;
					return ret;
				}
				c++;
			}
		}
		(*str)++;
	}
}

void unescape(char *str) {
	int i, j = 0, literal = 0;
	for(i = 0; str[i] != 0; i++) {
		if(j < i)
			str[j] = str[i];
		if(str[i] != '\\' || literal) {
			j++;
			if(literal)
				literal = 0;
		} else literal = 1;
	}
	str[j] = 0;
}

