#include "matwm.h"

void spawn(char *cmd) { /* run a command with sh -c */
	int pid = vfork();
	if(pid == 0) {
		if(vfork() == 0) { /* we fork twice here to prevent leaving zombie processes around */
			setsid();
			if(dn)
				setenv("DISPLAY", dn, 1);
			execlp("sh", "sh", "-c", cmd, (char *) 0);
		}
		else
			_exit(0);
	}
	if(pid > 0)
		wait(NULL);
}

int read_file(char *path, char **buf) { /* allocates memory at, and reads a file to *buf */
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

char *eat(char **str, char *until) { /* returns a pointer at the first non-whitespace character of *str, replaces the first occurence of one of the characters in null terminated string until by null and sets *str to the adress of the next character unless it encounters a null character in *str in wich case it sets *str to NULL */
	char *ret = NULL, *c;
	int literal = 0;
	while(1) {
		if(!ret && (**str != ' ' && **str != '\t')) /* ret is not set yet and **str is not whitespace, set ret */
			ret = *str;
		if(**str == '\\' && !literal) { /* escape character encountered */
			literal = 1;
			(*str)++;
			continue;
		}
		if(**str == 0) { /* null encountered, end of *str */
			*str = NULL;
			return ret;
		}
		if(literal) /* last character was an escape character, don't compare it to the characters in until and unset literal */
			literal = 0;
		else {
			c = until;
			while(ret && *c) {
  			if(**str == *c) { /* **str matches until, time to return */
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

void unescape(char *str) { /* to remove escape characters when we are no longer looking for special characters */
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

void *_malloc(size_t size) { /* malloc with error checking */
	void *r = malloc(size);
	if(!r)
		error();
	return r;
}

