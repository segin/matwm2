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

/* XXX: for now */

char *basename(char *arg)
{
	return arg;
}

void _start(char **args, void (*cleanup)(void), void *unused, void *unused2)
{
	int argc;
	char **argv;
	char **envp;
	
	argc = *(long *) args;
	argv = args + 1;
	envp = args + 2 + argc;
	environ = envp;
	
	__progname = argv[0] ? basename(argv[0]) : "";
	atexit(cleanup);
	exit(main(argc, argv, envp));
}
