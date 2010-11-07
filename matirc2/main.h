#ifndef __MAIN_H__
#define __MAIN_H__

#include "misc.h"

#define BUFFER_SIZE 1000000

typedef struct {
	char *data;
	int pos;
	bool wrap, changed;
} buffer_t;

typedef struct {
	char *name;
	buffer_t buffer;
	bool part;
} channel_t;

typedef struct {
	int socket, nchannels;
	char *hostname, *servname, *nick;
	buffer_t buffer;
	channel_t **channels;
} server_t;

extern server_t **servers, *server_current;
extern int nservers;
extern channel_t *channel_current;

void buffer_create(buffer_t *buffer);
void server_create(char *nick);
void server_destroy(server_t *s);
channel_t *channel_create(server_t *s, char *name);
void channel_destroy(server_t *s, channel_t *c);
void server_connect(server_t *s, char *hostname, char *servname, char *user_username, char *user_hostname, char *user_servername, char *user_realname);
void server_disconnect(server_t *s);
channel_t *channel_get(server_t *s, char *name);
void channel_focus(server_t *s, channel_t *c);

#endif /* __MAIN_H__ */
