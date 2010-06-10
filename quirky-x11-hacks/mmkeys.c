/* Copyright (c) 2010, Mattis Michel */
/* All rights reserved.              */

/* Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by Mattis Michel.
 * 4. Neither the name 'Mattis Michel' nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

struct key {
	int code;
	char *action;
} keys[] = {
	/* modify this */
	{244, "xterm"},
	{0, NULL} /* and keep this last */
};

#ifndef __progname
	#define __progname "mmkeys"
#endif

void spawn(char *cmd) { /* runs a command with sh -c */
	if(vfork() == 0) {
		setsid();
		execlp("sh", "sh", "-c", cmd, (char *) 0);
		_exit(0);
	}
}

void sigchldhandler(int i) {
	waitpid(WAIT_ANY, &i, WNOHANG);
}

int main(int argc, char *argv[]) {
	Display *dpy = XOpenDisplay(NULL);
	Window root;
	int i;
	if (!dpy) {
		fprintf(stderr, __progname ": error: cannot open display\n");
		exit(EXIT_FAILURE);
	}
	root = DefaultRootWindow(dpy);
	for (i = 0; keys[i].action; i++)
		XGrabKey(dpy, keys[i].code, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
	signal(SIGCHLD, &sigchldhandler);
	while (1) {
		XEvent ev;
		XNextEvent(dpy, &ev);
		if (ev.type == KeyPress)
			for (i = 0; keys[i].action; i++)
				if (ev.xkey.keycode == keys[i].code)
					spawn(keys[i].action);
	}
}
