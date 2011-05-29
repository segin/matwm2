/* KDE 'kshell' mini-clone.
 * This software is released under the ISC license. No nasty GPL applies anymore.
 * 
 * This program launches processes "daemonized", that is, with no controlling
 * tty... Well, really, it just kills the program's stdin/stdout/stderr before
 * it even launches the program.
 *  
 * Copyright (c) 2006, 2011, Kirn Gill <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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

