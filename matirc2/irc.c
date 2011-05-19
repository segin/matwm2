#include "irc.h"
#include "misc.h"

#include <sys/types.h>
#ifndef __WIN32__
#include <sys/uio.h>
#endif
#include <unistd.h>

void irc_alloc(irc_command *command, size_t size) {
	command->data = (char *) _realloc((void *) command->data, size);
	command->size = size;
}

int irc_receive(int socket, irc_command *command) {
	int pos, len, ret;
	bool last;
	start:
	pos = 0;
	len = 0;
	last = false;
	command->nick = NULL;
	command->nargs = 0;
	/* read a line */
	if(command->size != ALLOC_INCREMENT)
		irc_alloc(command, ALLOC_INCREMENT);
	while((ret = read(socket, command->data + len, 1)) == 1 && command->data[len] != '\n') {
		len++;
		if(len == command->size)
			irc_alloc(command, command->size + ALLOC_INCREMENT);
	}
	command->data[len] = 0;
	/* try to extract nickname */
	if(*command->data == ':') {
		for(; pos < len; pos++) {
			if(command->data[pos] == '!') {
				command->data[pos] = 0;
				command->nick = command->data + 1;
			}
			if(command->data[pos] == ' ') {
				pos++;
				break;
			}
		}
	}
	/* extract command and eventually arguments */
	command->command = command->data + pos;
	for(; pos < len && command->data[pos] != '\r' && command->data[pos] != '\n'; pos++)
		if((!last && command->data[pos] == ' ')) {
			command->data[pos] = 0;
			if(command->data[pos + 1] == ':') {
				last = true;
				pos++;
			}
			command->args[command->nargs] = command->data + pos + 1;
			command->nargs++;
		}
	command->data[pos] = 0;
	if(command->command && *command->command != 0)
		return 1;
	if(ret == 1)
		goto start;
	return 0;
}

void irc_initialize(irc_command *command) {
	command->data = NULL;
	command->size = 0;
}

void irc_free(irc_command *command) {
	free(command->data);
}
