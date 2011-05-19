#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "misc.h"
//#include "dprintf.h"

bool command_handle(char *str);
#ifdef __WIN32__
#define false 0
#define true -1
#endif

#endif /* __COMMANDS_H__ */
