#ifndef __IRC_H__
#define __IRC_H__

#define MAX_ARGS 16

typedef struct {
	char *nick, *command, *args[MAX_ARGS], *data;
	int nargs, size;
} irc_command;

int irc_receive(int socket, irc_command *command);
void irc_free(irc_command *command);

#endif /* __IRC_H__ */
