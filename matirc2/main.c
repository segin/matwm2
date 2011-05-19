#include "main.h"
#include "tcp.h"
#include "irc.h"
#include "misc.h"
#include "screen.h"
#include "dprintf.h"

#include <stdio.h> /* for perror() */
#include <unistd.h> /* for STDIN_FILENO and close() */
#include <string.h> /* for strcmp() and strcasecmp() */
#include <stdlib.h> /* for exit() */
#include <signal.h>
#include <errno.h>

server_t **servers = NULL, *server_current = NULL;
int nservers = 0;
channel_t *channel_current = NULL;
bool servers_changed;

void buffer_create(buffer_t *buffer) {
	buffer->data = (char *) _malloc(BUFFER_SIZE);
	buffer->data[0] = 0;
	buffer->pos = 0;
	buffer->wrap = false;
	buffer->changed = false;
}

void server_create(char *nick) {
	server_t *s;
	servers = (server_t **) _realloc((void *) servers, sizeof(server_t *) * (nservers + 1));
	servers[nservers] = (server_t *) _malloc(sizeof(server_t));
	s = servers[nservers];
	buffer_create(&s->buffer);
	s->nick = string_copy(nick);
	s->hostname = NULL;
	s->servname = NULL;
	s->channels = NULL;
	s->nchannels = 0;
	s->socket = -1;
	nservers++;
	channel_focus(s, NULL);
}

void server_destroy(server_t *s) {
	int i, snum;
	bool shift = false;
	if(s->socket != -1)
		server_disconnect(s);
	free(s->hostname);
	free(s->servname);
	free(s->nick);
	free(s->buffer.data);
	for(i = 0; i < s->nchannels; i++)
		channel_destroy(s, s->channels[i]);
	free(s->channels);
	for(i = 0; i < nservers; i++) {
		if(shift)
			servers[i - 1] = servers[i];
		if(servers[i] == s) {
			shift = true;
			snum = i;
		}
	}
	free(s);
	nservers--;
	servers = (server_t **) _realloc((void *) servers, sizeof(server_t *) * nservers);
	if(!nservers)
		server_create(getenv("USER"));
	if(server_current == s) {
		s = servers[snum ? snum - 1 : 0];
		channel_focus(s, s->nchannels ? s->channels[s->nchannels - 1] : NULL);
	}
}

channel_t *channel_create(server_t *s, char *name) {
	channel_t *c;
	s->channels = (channel_t **) _realloc((void *) s->channels, sizeof(channel_t *) * (s->nchannels + 1));
	s->channels[s->nchannels] = (channel_t *) _malloc(sizeof(channel_t));
	c = s->channels[s->nchannels];
	c->name = string_copy(name);
	buffer_create(&c->buffer);
	c->part = false;
	s->nchannels++;
	channel_focus(server_current, c);
	return c;
}

void channel_destroy(server_t *s, channel_t *c) {
	int i, cnum;
	bool shift = false;
	if(!c) {
		server_destroy(s);
		return;
	}
	if(channel_current)
		if(channel_current->part)
			dprintf(server_current->socket, "PART :%s\r\n", channel_current->name);
	free(c->name);
	free(c->buffer.data);
	for(i = 0; i < s->nchannels; i++) {
		if(shift)
			s->channels[i - 1] = s->channels[i];
		if(s->channels[i] == c) {
			shift = true;
			cnum = i;
		}
	}
	free(c);
	s->nchannels--;
	s->channels = (channel_t **) _realloc((void *) s->channels, sizeof(channel_t *) * s->nchannels);
	if(channel_current == c)
		channel_focus(s, cnum ? s->channels[cnum - 1] : NULL);
}

void server_connect(server_t *s, char *hostname, char *servname, char *user_username, char *user_hostname, char *user_servername, char *user_realname) {
	s->hostname = string_copy(hostname);
	s->servname = string_copy(servname);
	s->socket = tcp_connect(hostname, servname);
	if(s->socket == -1) {
		screen_printf(s, NULL, "* cannot connect: %s\n", tcp_error);
	} else {
		dprintf(s->socket, "NICK %s\r\n", s->nick);
		dprintf(s->socket, "USER %s %s %s :%s\r\n", user_username, user_hostname, user_servername, user_realname);
	}
	screen_status_update();
	servers_changed = true;
}

void server_disconnect(server_t *s) {
	dprintf(s->socket, "QUIT\r\n");
	close(s->socket);
	s->socket = -1;
	servers_changed = true;
}

channel_t *channel_get(server_t *s, char *name) {
	int i;
	for(i = 0; i < s->nchannels; i++)
		if(strcmp(name, s->channels[i]->name) == 0)
			return s->channels[i];
	return NULL;
}

void handle_command(server_t *s, irc_command *c) {
	int i;
	channel_t *ch;
	if(strcasecmp(c->command, "ping") == 0 && c->nargs) {
		dprintf(s->socket, "PONG :%s\r\n", c->args[0]);
	} else if(strcasecmp(c->command, "nick") == 0 && c->nick && c->nargs) {
		if(strcmp(c->nick, s->nick) == 0) {
			string_recopy(&s->nick, c->args[0]);
			screen_entry_update();
		}
	} else if(strcasecmp(c->command, "notice") == 0 && c->nargs > 1) {
		screen_printf(s, NULL, "* notice: %s\n", c->args[1]);
	} else if(strcasecmp(c->command, "join") == 0 && c->nick && c->nargs) {
		if(strcmp(c->nick, s->nick) == 0) {
			if(!(ch = channel_get(s, c->args[0])))
				ch = channel_create(s, c->args[0]);
			screen_printf(s, ch, "* joined channel %s\n", c->args[0]);
			ch->part = true;
			return;
		} else screen_printf(s, channel_get(s, c->args[0]), "* %s has joined the channel\n", c->nick);
	} else if(strcasecmp(c->command, "part") == 0 && c->nick && c->nargs > 1) {
		screen_printf(s, channel_get(s, c->args[0]), "* %s has left the channel (%s)\n", c->nick, c->args[1]);
	} else if(strcasecmp(c->command, "privmsg") == 0 && c->nick && c->nargs > 1) {
		if(strcmp(c->args[1], "VERSION") == 0) {
			screen_printf(s, NULL, "* CTCP VERSION from %s\n", c->nick);
			dprintf(s->socket, "NOTICE %s :VERSION %s\r\n", c->nick, "matirc 0.0.0");
			return;
		}
		if(strcmp(c->args[0], s->nick) == 0) {
			if(!(ch = channel_get(s, c->args[0])))
				ch = channel_create(s, c->args[0]);
			screen_printf(s, ch, "<%s> %s\n", c->nick, c->args[1]);
			return;
		}
		screen_printf(s, channel_get(s, c->args[0]), "<%s> %s\n", c->nick, c->args[1]);
	} else if(strcasecmp(c->command, "mode") == 0) {
		if(c->nargs == 2)
			screen_printf(s, NULL, "* mode %s for %s\n", c->args[1], c->args[0]);
		else if(c->nargs == 3 && c->nick)
			screen_printf(s, channel_get(s, c->args[0]), "* %s sets mode %s for %s\n", c->nick, c->args[1], c->args[2]);
	} else if(strcasecmp(c->command, "topic") == 0 && c->nargs > 1) {
		screen_printf(s, channel_get(s, c->args[0]), "* %s has changed the topic to %s\n", c->nick, c->args[1]);
	} else if(strcmp(c->command, "332") == 0 && c->nargs > 1) { /* topic for joined channel */	
		screen_printf(s, channel_get(s, c->args[1]), "* topic: %s\n", c->args[2]);
	} else if(strcmp(c->command, "353") == 0 && c->nargs > 1) { /* /names list */	
		ch = channel_get(s, c->args[2]);
		if(ch)
			screen_printf(s, ch, "* names: %s\n", c->args[3]);
		else screen_printf(s, NULL, "* names for %s: %s\n", c->args[2]);
	} else if(strcmp(c->command, "372") == 0 && c->nargs > 1) { /* motd */	
		screen_printf(s, NULL, "* motd: %s\n", c->args[1]);
	} else if(strcmp(c->command, "333") != 0 && strcmp(c->command, "366") != 0 && strcmp(c->command, "376") != 0) {
		screen_printf(s, NULL, "* unrecognised message: %s: %s", c->nick, c->command);
		for(i = 0; i < c->nargs; i++)
			screen_printf(s, NULL, " '%s'", c->args[i]);
		screen_printf(s, NULL, "\n");
	}
}

void channel_focus(server_t *s, channel_t *c) {
	buffer_t *b = c ? &c->buffer : &s->buffer;
	b->changed = false;
	server_current = s;
	channel_current = c;
	screen_redraw();
}

void quit(void) {
	int i;
	for(i = 0; i < nservers; i++)
		server_disconnect(servers[i]);
#ifdef __WIN32__
	WSACleanup();
#endif /* __WIN32__ */
	screen_end();
}

void sighandler(int sig) { exit(EXIT_SUCCESS); }

int main(int argc, char *argv[]) {
	irc_command command;
	fd_set fds, fdsc;
	int i, j, last;

#ifdef __WIN32__ 
	WSADATA WSAData;
 
	i = WSAStartup(WINSOCK_VERSION, &WSAData);
	if(i != 0)
	{
		printf("[-] Error in main() with WSAStartup(): %d\n", i);
		return 0;
	}
#else 
	signal(SIGHUP, &sighandler);
	signal(SIGUSR1, &sighandler);
	signal(SIGCHLD, &sighandler);
	signal(SIGINT, &sighandler);
#endif /* __WIN32__ */
	signal(SIGTERM, &sighandler);

	atexit(&quit);
	screen_initialize();

	irc_initialize(&command);
	server_create(getenv("USER"));

	start:
	servers_changed = false;
	last = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	for(i = 0; i < nservers; i++) {
		if(servers[i]->socket == -1)
			continue;
		FD_SET(servers[i]->socket, &fds);
		if(servers[i]->socket > last)
			last = servers[i]->socket;
	}
	fdsc = fds;
	while(1) {
		if(select(last + 1, &fdsc, NULL, NULL, NULL) == -1) {
			if(errno == EINTR) {
				#ifdef SIGWINCH
				if(screen_resized)
					screen_update_size();
				#endif
				goto end_loop;
			}
			_perror("select()");
		}
		for(i = 0; i < nservers; i++)
			if(servers[i]->socket != -1)
			 	if(FD_ISSET(servers[i]->socket, &fdsc)) {
					if(!irc_receive(servers[i]->socket, &command)) {
						screen_printf(servers[i], NULL, "*** disconnected\n");
						for(j = 0; j < servers[i]->nchannels; j++)
							screen_printf(servers[i], servers[i]->channels[j], "*** disconnected\n");
						servers[i]->socket = -1;
						goto start;
					}
					handle_command(servers[i], &command);
				}
		if(FD_ISSET(STDIN_FILENO, &fdsc))
			screen_gotch(wgetch(screen_entry));
		if(servers_changed)
			goto start;
		end_loop:
		fdsc = fds;
	}
}
