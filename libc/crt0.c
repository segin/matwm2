/* crt0.c: C runtime entry point 
 * Written by Kirn Gill <segin2005@gmail.com>
 */
 
char* __progname;
char** environ;

#ifndef NULL
# define NULL (void *) 0x0
#endif

extern int main(int argc, char **argv, **envp);

void _start(int argc, char **argv, char **env)
{
	environ = envp;
	argv[0] ? __progname = argv[0] : __progname = NULL;
	exit(main(argc, argv, envp));
}
