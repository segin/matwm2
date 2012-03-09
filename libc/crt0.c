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

void _start(char **ap, void (*cleanup)(void), void *unused, void *unused2)
{
	int argc;
	char **argv;
	char **envp;
	
	argc = * (long *) ap;
	argv = ap + 1;
	envp = ap + 2 + argc;
	
	environ = envp;
	*argv[0] ? __progname = argv[0] : __progname = "";
	exit(main(argc, argv, envp));
}
