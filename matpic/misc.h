#ifndef __MISC_H__
#define __MISC_H__

void errexit(char *msg);
void flerrexit(char *file, int line, char *msg);
void flwarn(char *file, int line, char *msg);
void fawarn(char *file, int addr, char *msg);

#endif /* __MISC_H__ */

