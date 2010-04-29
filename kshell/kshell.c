/* KDE 'kshell' mini-clone.
 * This software is released under both the GNU GPL version 2 and BSD licenses.
 * 
 * This program launches processes "daemonized", that is, with no controlling
 * tty... Well, really, it just kills the program's stdin/stdout/stderr before
 * it even launches the program.
 *  
 * Copyright (C) 2006 Segin
 * 
 * Version 1.1, 2006 May 15
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void run_program(char **argv)
{
	FILE *tty;
	char *ttypath = malloc(4096 * sizeof(char));

	strcpy(ttypath,ttyname(1));

	daemon(1,0);

	execvp(argv[0],argv);
	
	tty = fopen(ttypath,"w");
	free(ttypath);

	fprintf(tty, "Process failed to launch.\n");
	exit(1);
}

int main(int argc, char **argv)
{
	pid_t pid;
	char *str;
	
	if (argc == 1) {
		printf("usage: kshell command [args]\n");
		exit(1);
	}
	
	argv++;
	argc--;
	
	pid = fork();
	
	if (pid == 0) {
		run_program(argv);
	
	}
	
	printf("process launched, pid = %d.\n",pid);
	exit(0);
}

