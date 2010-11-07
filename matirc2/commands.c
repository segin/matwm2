#include "commands.h"
#include "main.h"
#include "screen.h"

#include <string.h> /* for strcasecmp() and strlen() */
#include <stdlib.h> /* for exit() */

char *args[16];
int nargs = 0;

char *strip(char **ptr) {
	char *ret = *ptr;
	while(**ptr) {
		if(**ptr == ' ' || **ptr == '\t') {
			**ptr = 0;
			(*ptr)++;
			if(!**ptr)
				*ptr = NULL;
			return ret;
		}
		(*ptr)++;
	}
	*ptr = NULL;
	return ret;
}

int mstrip(char **ptr, int n) {
	nargs = 0;
	while(*ptr && nargs < n - 1) {
		args[nargs] = strip(ptr);
		nargs++;
	}
	if(*ptr)
		args[nargs++] = *ptr;
	return nargs;
}

bool command_handle(char *str) {
	char *ptr = string_copy(str), *cmd = strip(&ptr);
	if(strcasecmp(cmd, "connect") == 0) {
		if(!ptr && server_current->hostname && server_current->servname) {
			server_connect(server_current, server_current->hostname, server_current->servname, getenv("USER"), getenv("HOST"), "*", "matirc user");
			goto cmd_ok;
		}
		if(!ptr)
			screen_printf(server_current, channel_current, "* no server set\n");
		goto server;
	}
	if(strcasecmp(cmd, "server") == 0) {
		if(!ptr)
			screen_printf(server_current, channel_current, "Usage: SERVER <hostname> [<port>]\n");
		else {
			server:
			mstrip(&ptr, 2);
			server_connect(server_current, args[0], (nargs == 2) ? args[1] : "6667", getenv("USER"), getenv("HOST"), "*", "matirc user");
		}
		goto cmd_ok;
	}
	if(strcasecmp(cmd, "disconnect") == 0) {
		if(server_current->socket != -1)
			server_disconnect(server_current);
		goto cmd_ok;
	}
	if(strcasecmp(cmd, "msg") == 0) {
		if(mstrip(&ptr, 2) == 2)
			dprintf(server_current->socket, "PRIVMSG %s :%s\r\n", args[0], args[1]);
		else if(nargs == 1)
			channel_create(server_current, args[0]);
		else screen_printf(server_current, channel_current, "Usage: MSG <nick> [<message>]\n");
		goto cmd_ok;
	}
	if(strcasecmp(cmd, "nick") == 0) {
		if(!ptr)
			screen_printf(server_current, channel_current, "Usage: NICK <nickname>\n");
		else {
			string_recopy(&server_current->nick, ptr);
			screen_entry_update();
			if(server_current->socket != -1)
				dprintf(server_current->socket, "NICK :%s\r\n", ptr);
		}
		goto cmd_ok;
	}
	if(strcasecmp(cmd, "names") == 0 && !ptr && channel_current) {
		dprintf(server_current->socket, "NAMES :%s\r\n", channel_current->name);
		goto cmd_ok;
	}
	if(strcasecmp(cmd, "ctcp") == 0) {
		if(mstrip(&ptr, 2) == 2)
			dprintf(server_current->socket, "PRIVMSG %s :%s\r\n", args[0], args[1]);
		else if(nargs == 1)
			channel_create(server_current, args[0]);
		else screen_printf(server_current, channel_current, "Usage: CTCP <nick> <command>\n");
		goto cmd_ok;
	}
	free(cmd);
	return false;
	cmd_ok:
	free(cmd);
	return true;
}
