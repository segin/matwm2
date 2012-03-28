/* crt0.c: C runtime entry point 
 * Written by Kirn Gill <segin2005@gmail.com>
 */
 
#include <stdlib.h>
 
char* __progname;
char** environ;

#ifndef NULL
# define NULL (void *) 0x0
#endif

extern int main(int argc, char **argv, char **envp);

char *_crt0_basename(char *name)
{
	const char *base = name;
	while(*name)
		if (*name++ == '/') 
			base = name;
	return base;
}

void _start(char **args, void (*cleanup)(void), ...)
{
	int argc;
	char **argv;
	char **envp;
	
	argc = *(long *) args;
	argv = args + 1;
	envp = args + 2 + argc;
	environ = envp;
	
	__progname = argv[0] ? _crt0_basename(argv[0]) : "";
	if (cleanup) atexit(cleanup); /* ld.so cleanup function */
	exit(main(argc, argv, envp));
}
