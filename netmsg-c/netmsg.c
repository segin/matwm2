/* 
 * NetMSG Protocol, C Language Implementation 
 * Copyright (C) 2008-2009 Kirn Gill <segin2005@gmail.com>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE. 
 */
 
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <curses.h>
#include "netmsg.h"
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/utsname.h>

#include <strings.h>
#include <sys/types.h>
/* The networking header is winsock.h on Windows. */
#ifdef __WIN32__
#include <winsock2.h>
#else /* For Unix */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#define VERSION			"20081231"
#define VERSION_NUMERIC		20081231
WINDOW *inputwin, *outputwin, *statuswin;
char *host, *user;
pthread_t statusthrd, msgthrd;
int s; /* Global socket */

void resize(int a);
void startgui(void);
void wordwrap(char *s);
int tcp_connect(char *host, int port);
int tcp_listen(int port);
void quit(void);
void endgui(void);
void guiprintf(char *fmt, ...);
void debug_message(char *msg);
void debug_message_fmt(char *fmt, ...);
#ifdef _NEED_STRSEP
/* GPL-Tainted */
char* strsep (char **string, const char *delim);
#endif 
void set_window_color(WINDOW *win, int color);
void set_color(int color);

#if defined (SIGWINCH)
void resize(int a)
{
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	resizeterm(size.ws_row, size.ws_col);
	int maxy, maxx;
	getmaxyx(stdscr, maxy, maxx);
	wresize(inputwin, 1, maxx);
	wresize(outputwin, maxy - 1, maxx);
	wresize(statuswin, 1, maxx);
	mvwin(inputwin, maxy - 1, 0);
	mvwin(inputwin, maxy - 2, 0);
	wclear(outputwin);
	wrefresh(outputwin);
	wrefresh(inputwin);
	wrefresh(statuswin);
	signal(SIGWINCH, resize);
}
#else

#define SIGWINCH 28 
void resize(int a)
{
	signal(SIGWINCH, resize);
}
#endif

void startgui()
{
	int maxy, maxx;
	initscr();
	if(has_colors()) {
		start_color();
#ifdef WANT_PAINTED_GUI
#ifdef assume_default_colors
		assume_default_colors(COLOR_WHITE, COLOR_BLACK);
#endif
		init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
		init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
		init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
		init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
		init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
		init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
		init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
#endif
	}
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	getmaxyx(stdscr, maxy, maxx);
	outputwin = newwin(maxy - 1, 0, 2, 0);
	inputwin = newwin(1, 0, 0, 0);
	statuswin = newwin(1, 0, 1, 0);
	scrollok(outputwin, 1);
	scrollok(inputwin, 1);
	scrollok(statuswin, 0);
	wattron(statuswin,A_REVERSE);
#if defined(SIGWINCH) && !defined(NORESIZE)
	signal(SIGWINCH, resize);
#endif
}

void
strcatl(char *dst, const char *src, size_t size)
{
	int n, cnt;

	n = strlen(dst);
	cnt = size-1-n;
	if(cnt > 0)
		strncat(dst,src,cnt);
}

void update_status(void)
{
	char *str = malloc(4096), *str2 = malloc(4096);
	int len, maxlen, maxx, maxy;
	getmaxyx(stdscr, maxy, maxx);
	wclear(statuswin);
	snprintf(str, 4096, " -- Connected to %s (%s) [client] --",
			host,user);
	len = strlen(str);
	maxlen = maxx - len;
	bzero(str2, 4096);
	memset(str2, 32, maxlen);
	strcat(str, str2);
	waddstr(statuswin, str);
	free(str);
	free(str2);
	wrefresh(statuswin);
	//wrefresh(inputwin);
}

void
wordwrap(char *s)
{
	int mcol, row, col;
	getyx(outputwin, row, col);
	char *ptr = strchr(s,'\0');
	while(ptr>s)
	{
		ptr--;
		if((*ptr == '\n')|| (*ptr == ' '))
			*ptr = '\0';
		else break;
	}
	char *buf;
	if(!(buf = (char *)calloc(2048, sizeof(char))))
		/* err(1,NULL); */
		guiprintf("");
	mcol = COLS-1;
	int cols = mcol - col;
	ptr = s;
	while(strlen(ptr) > cols)
	{
		if(strchr(ptr,'\n')) break;
		char *spc = ptr+cols;
		while(spc > ptr)
		{
			if(*spc == ' ')
				break;
			else
				--spc; /* back up a byte till we find space */
		}
		if(spc > ptr) 
		{
			int n = spc - ptr;
			strncat(buf,ptr,n);
			strcatl(buf,"\n",2048);
			ptr = spc+1; /* skip it */
			while(*ptr && (*ptr == ' ')) ++ptr;
			cols = mcol;
		}
		else
		{
			strncat(buf,ptr,cols);
			strcatl(buf,"\n",2048);
			ptr = ptr+(cols);
			while(*ptr && (*ptr == ' ')) ++ptr;
			cols = mcol;
		}
	}
	if(ptr == s) /* maybe we didnt need to wordwrap */
	{
		wprintw(outputwin,"%s",s);
	}
	else if(*ptr != '\0') /* the left overs from the previous while loop */
	{
		strcatl(buf,ptr,2048);
		wprintw(outputwin,"%s",buf);
	}
	free(buf);
}

void guiprintf(char *fmt, ...)
{
	int maxx, maxy;
	char *text = malloc(4097); /* Assume malloc() will succeed */
	va_list ap;
	getmaxyx(stdscr, maxy, maxx);
	va_start(ap, fmt);
	vasprintf(&text, fmt, ap);
	va_end(ap);
	wprintw(outputwin, text);
	free(text);
	wrefresh(outputwin);
	wrefresh(inputwin);
}

/* Big #ifdef because of broken Winsock */

int tcp_connect(char *host, int port)
#ifdef __WIN32__
{
 SOCKET sock;
 sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
 if(sock==INVALID_SOCKET) {
    return -1;
}
  struct hostent *remotehost;
  if((remotehost = gethostbyname(host)) == 0) return -2;
  struct sockaddr_in addr;
  bzero(addr.sin_zero, 8);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = *((struct in_addr *) remotehost->h_addr);
  if(connect(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr)) == -1) return -3;
  return sock;
}
#else
{
  struct protoent *proto = getprotobyname("tcp");
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, proto->p_proto)) == -1)
    return -1;
  struct hostent *remotehost;
  if((remotehost = gethostbyname(host)) == 0)
    return -2;
  struct sockaddr_in addr;
  bzero(addr.sin_zero, 8);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = *((struct in_addr *) remotehost->h_addr);
  if(connect(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr)) == -1)
    return -3;
  return sock;
}
#endif
int tcp_listen(int port)
{
#ifdef __WIN32__
 SOCKET sock;
 sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
 if(sock==INVALID_SOCKET)
#else
  struct protoent *proto = getprotobyname("tcp");
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, proto->p_proto)) == -1)
#endif
    return -1;
  int reuseaddr = 1;
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1)
    return -2; 
  struct sockaddr_in localsockaddr;
  bzero(localsockaddr.sin_zero, sizeof(localsockaddr.sin_zero));
  localsockaddr.sin_family = AF_INET;
  localsockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localsockaddr.sin_port = htons(port);
  if(bind(sock, (struct sockaddr *) &localsockaddr, sizeof(localsockaddr)) == -1)
    return -3;
  if(listen(sock, SOMAXCONN) == -1)
    return -4;
  return sock;
}

/* Debugging messages are green because the vanilla NetBSD kernel's 
 * messages appear in green, and I am a rather big fan of NetBSD.
 */
void debug_message(char *msg)
{
	wattron(outputwin, COLOR_PAIR(COLOR_GREEN));
	guiprintf("DEBUG: %s\n",msg);
	wattron(outputwin, COLOR_PAIR(COLOR_WHITE));
}

void debug_message_fmt(char *fmt, ...)
{
	char *msg;
	va_list ap;
	wattron(outputwin, COLOR_PAIR(COLOR_GREEN));
	va_start(ap, fmt);
	vasprintf(&msg, fmt, ap);
	va_end(ap);
	guiprintf("DEBUG: %s\n",msg);
	free(msg);
	wattron(outputwin, COLOR_PAIR(COLOR_WHITE));
}

void quit(void)
{
	endgui();
	exit(0);
}

void endgui(void)
{
	// pthread_kill(statusthrd,SIGKILL);
	delwin(outputwin);
	delwin(statuswin);
	delwin(inputwin);
	endwin();
}

void set_window_color(WINDOW *win, int color)
{
	wattron(win, COLOR_PAIR(color));
}

void set_color(int color)
{
	wattron(outputwin, COLOR_PAIR(color));
}

void display_version(void)
{
	struct utsname sysinfo;
	uname(&sysinfo);
	guiprintf("NetMSG version ");
	set_color(COLOR_BLUE);
	guiprintf("%s", VERSION);
	set_color(COLOR_WHITE);
	guiprintf(", running on ");
	set_color(COLOR_CYAN);
	guiprintf("%s %s %s\n", sysinfo.machine, sysinfo.sysname, 
		  sysinfo.release);
	set_color(COLOR_WHITE);
}

/* Command processor */
void command(char *cmd, char *arg)
{
	debug_message_fmt("command(\"%s\", \"%s\");", cmd, arg);
	if(strcasecmp(cmd, "quit") == 0 || strcasecmp(cmd, "q") == 0)
		quit();
	if(strcasecmp(cmd, "ver") == 0 || strcasecmp(cmd, "version") == 0)
		display_version();
}



void getusername()
{
	guiprintf("Enter a nickname.\n");
	char *txt; int ret;
	while(guigetln(&txt, 1, "nick") == 0);
	user = malloc(strlen(txt));
	strcpy(user,txt);
	free(txt);
	update_status();
}

void getnhostname()
{
	guiprintf("Enter the hostname or IP to connect to.\n");
	char *txt;
	int ret = 0 ;	
	while (ret == 0) {
		ret = guigetln(&txt, 1, "server");
		if (ret != 0) break;
	}
	host = malloc(strlen(txt));
	strcpy(host,txt);
	free(txt);
	update_status();
}

void recvthread(void *arg)
{
	s = (int) arg;
	int type;
	int size;
	struct netmsg_protocol packet;
	for(;;) {
		size = recv(s, &packet, sizeof(netmsg_protocol), 0);
		if (size != sizeof(netmsg_protocol)) {
			if (size == -1 || size == 0) {
				/* Server has died without telling us... */
				endgui();
				printf("netmsg: %s: Connection closed by "
				       "remote host.\n", host);
				exit(0);
			}
		}
		type = (int) *packet.p_type;
		if (type == PROTO_MSG_CHAT) {
			char *msg;
			asprintf(&msg, "%s: %s", packet.p_user, 
					packet.p_msg);
			set_color(COLOR_YELLOW);
			wordwrap(msg);
			set_color(COLOR_WHITE);
			free(msg);
		}
	}
}

int guigetln(char **text, int echo, char *msg)
{
  int c, len = 0;
  *text = (char *) malloc(1 * sizeof(char));
  if (msg == 0) { 
  	msg = malloc(2);
  	strcpy(msg, "");
  } else {
	  char *tmp;
	  asprintf(&tmp,"%s> ",msg);
	  msg = malloc(strlen(tmp) + 1);
	  strcpy(msg, tmp);
	  free(tmp);
  }
  waddstr(inputwin, msg);
  free(msg);
  while(c = wgetch(inputwin))
    {
      if(c == '\n')
        {
          (*text)[len] = 0;
          if(echo != 0)
            wclear(inputwin);
          return len;
        }
      else if(c == KEY_LEFT)
        {
          int y, x;
          getyx(inputwin, y, x);
          mvwdelch(inputwin, y, x - 1);
        }
      else if(c == KEY_BACKSPACE || c == erasechar() || c == 8)
        {
          if(len == 0)
            continue;
          len--;
	  (*text)[len] = 0;
          *text = (char *) realloc((char *) *text, (len + 1) * sizeof(char));
          if(echo != 0)
            {
              int y, x;
              getyx(inputwin, y, x);
              mvwdelch(inputwin, y, x - 1);
            }
        }
      else if(c == 15)
      {
	      wrefresh(outputwin);
	      wrefresh(inputwin);
	      wrefresh(statuswin);
      }
      else if(c >= 32)
        {
          if(echo != 0)
            waddch(inputwin, c);
          (*text)[len] = c;
          len++;
	  *text = (char *) realloc((char *) *text, (len + 1) * sizeof(char));
        }
    }
}

#ifdef _NEED_STRSEP
#define GPL_TAINTED
/* This strsep function is GPL-licensed... */
char* strsep (char **string, const char *delim)
{
  char *p, *q;
  
  if (!(p =* string))
    return NULL;
  
  if (delim[0] == '\0' || delim[1] == '\0')
    {
      char ch = delim[0];
      
      if (ch != '\0')
        q = (*p == ch) ? p : strchr (p + 1, ch);
      else
       q = NULL;
    }
  else
    q = strpbrk (p, delim);
  
  if (q)
    {
      *q++ = '\0';
      *string = q;
    }
  else
    *string = NULL;
  
  return p;
}
#endif

void status_thread()
{
	for(;;) {
		update_status();
		sleep(1);
	}
}

void send_proto_message(int type, char *msg)
{
	struct netmsg_protocol out;
	bzero(&out, sizeof(netmsg_protocol));
	*(int *) out.p_type = type;
	memcpy(out.p_user,user,strlen(user) >= 31 ? 31 : strlen(user)+1 );
	memcpy(out.p_msg,msg,strlen(msg) >= 256 ? 256 : strlen(msg)+1);
	send(s, &out, sizeof(netmsg_protocol), 0);
}

int main()
{
	int sock;
#ifdef __WIN32__
	struct WSAData wsaData;
	int wsaret=WSAStartup(0x101,&wsaData);
	if(wsaret) return;
#endif
	startgui();
	guiprintf("NetMSG %s. Running as PID %d\n",VERSION,getpid());
	pthread_create(&statusthrd, 0, status_thread, 0);
//	Get the user's name.
	getusername();
	getnhostname();
	guiprintf("Connecting to host %s on port 1337\n", host);
	sock = tcp_connect(host, 1337);
	if (sock != 0) {
		guiprintf("Connected to host %s.\n", host);
	} else {
		endwin();
		printf("Unable to connect to host %s: %s\n", host, 
				strerror(errno));
		exit(1);
	}
	pthread_create(&msgthrd, 0, recvthread, (void *) sock);
	for(;;) {
		char *txt;
		if(guigetln(&txt, 1, 0) == 0) {
			free(txt);
			continue;
		}
		if(txt[0] == '/') {
			/* Please explain the following:
			main () at netmsg.c:408
			408                     if(txt[0] == '/') {
			(gdb) print txt
			$2 = 0x804d490 "/quit test"
			(gdb) s
			409        char *txt2 = txt + 1;
			(gdb) print txt2
			$3 = 0x804d496 "test"
			(gdb) 
			*/
			char *txt2 = &txt[1]; 
			if(txt[1] == '\0') continue;
			char *cmd = strsep(&txt2, " ");
			command(cmd, txt2);
		} else
			debug_message(txt);
			send_proto_message(PROTO_MSG_CHAT, txt);
		free(txt);
	}
	quit();
}
