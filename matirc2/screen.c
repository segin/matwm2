#include "screen.h"
#include "main.h"
#include "commands.h"

#include <strings.h>
#include <stdio.h> /* for vasprintf(), dprintf() and fprintf() */
#include <curses.h>
/* for determining the terminal size */
#ifndef __WIN32__
#include <sys/ioctl.h>
#endif
#include <unistd.h> /* STDOUT_FILENO */

#ifdef __WIN32__
#define false 0
#define true -1
#endif

WINDOW *screen_main, *screen_status, *screen_entry;
int attrs = 0, screen_entry_pos = 0, screen_entry_len = 0, screen_history_start = -1, screen_history_pos = -1, screen_history_len = 0, screen_entry_offset = 0, screen_entry_scroll = 0;
char screen_entry_data[ENTRY_SIZE], *screen_history[HISTORY_SIZE], *screen_entry_copy = NULL;
struct winsize size;
#ifdef SIGWINCH
bool screen_resized = true;

void sigwinch(int s) {
	screen_resized = true;
}
#endif

#ifdef __WIN32__
void screen_get_win32_console_size(struct winsize *ws)
{
	/* XXX: Just assume 80x25 for now */
	ws->ws_row = 25;
	ws->ws_col = 80;
}
#endif

void screen_initialize(void) {
	initscr();
	noecho();
	cbreak();
#ifndef __WIN32__
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
#else
	screen_get_win32_console_size(&size);
#endif
	screen_main = newwin(size.ws_row - 2, size.ws_col, 0, 0);
	screen_status = newwin(1, size.ws_col, size.ws_row - 2, 0);
	screen_entry = newwin(1, size.ws_col, size.ws_row - 1, 0);
	scrollok(screen_main, TRUE);
	wbkgdset(screen_status, A_REVERSE);
	wclear(screen_status);
	keypad(screen_entry, TRUE);
	refresh();
	wrefresh(screen_status);
	#ifdef SIGWINCH
	signal(SIGWINCH, &sigwinch);
	#endif
}

#ifdef SIGWINCH
void screen_update_size(void) {
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  resize_term(size.ws_row, size.ws_col);
	wresize(screen_main, size.ws_row - 2, size.ws_col);
	mvwin(screen_status, size.ws_row - 2, 0);
	wresize(screen_status, 1, size.ws_col);
	mvwin(screen_entry, size.ws_row - 1, 0);
	wresize(screen_entry, 1, size.ws_col);
	screen_redraw();
	screen_resized = false;
}
#endif

void screen_end(void) {
	endwin();
}

void screen_addnstr(char *str, size_t len) {
	size_t p;
	for(p = 0; p < len; p++) {
		if(str[p] == '') {
			attrs ^= A_BOLD;
			wattrset(screen_main, attrs);
			continue;
		}
		waddch(screen_main, str[p]);
		if(str[p] == '\n') {
			attrs = 0;
			wattrset(screen_main, attrs);
		}
	}
}

void screen_printf(server_t *s, channel_t *c, char *format, ...) {
	va_list ap;
	char *str;
	buffer_t *buffer = c ? &c->buffer : &s->buffer;
	int len;
	va_start(ap, format);
	len = vasprintf(&str, format, ap);
	va_end(ap);
	if((c && c == channel_current) || (c == NULL && channel_current == NULL && s == server_current)) {
		if(buffer->pos || buffer->wrap)
			if(buffer->data[buffer->pos ? buffer->pos - 1 : BUFFER_SIZE - 1] == '\n')
				screen_addnstr("\n", 1);
		screen_addnstr(str, len - ((str[len - 1] == '\n') ? 1 : 0));
		wrefresh(screen_main);
		wrefresh(screen_entry);
	} else if(!buffer->changed) {
		buffer->changed = true;
		screen_status_update();
	}
	while(*str) {
		buffer->data[buffer->pos] = *str;
		*str++;
		buffer->pos = (++buffer->pos < BUFFER_SIZE) ? buffer->pos : 0;
		if(buffer->pos == 0 && buffer->wrap == false)
			buffer->wrap = true;
	}
}

void screen_status_update(void) {
	int i, j;
	wclear(screen_status);
	for(i = 0; i < nservers; i++) {
		if(channel_current == NULL && server_current == servers[i])
			wattron(screen_status, A_BOLD);
		wprintw(screen_status, " [%c%s ]", servers[i]->buffer.changed ? '+' : ' ', servers[i]->hostname);
		wattroff(screen_status, A_BOLD);
		for(j = 0; j < servers[i]->nchannels; j++) {
			if(channel_current == servers[i]->channels[j])
				wattron(screen_status, A_BOLD);
			wprintw(screen_status, " [%c%s ]", servers[i]->channels[j]->buffer.changed ? '+' : ' ', servers[i]->channels[j]->name);
			wattroff(screen_status, A_BOLD);
		}
	}
	wrefresh(screen_status);
	wrefresh(screen_entry);
}

int screen_text_size(char *str, int start, int len) {
	int i;
	for(i = start; i < len; i++) {
		if(str[i] < 31)
			len++;
		if(str[i] == '\t')
			len += TABSIZE - 1;
	}
	return len;
}

int screen_entry_pos_real(void) {
	return screen_text_size(screen_entry_data, screen_entry_scroll, screen_entry_pos) + screen_entry_offset - screen_entry_scroll;
}

void screen_entry_update(void) {
	int y, x;
	wclear(screen_entry);
	wprintw(screen_entry, "[ %s ] ", server_current->nick) - 1;
	getyx(screen_entry, y, screen_entry_offset);
	waddnstr(screen_entry, screen_entry_data + screen_entry_scroll, screen_entry_len - screen_entry_scroll);
	wmove(screen_entry, 0, screen_entry_pos_real());
	wrefresh(screen_entry);
}

void screen_entry_scroll_right(void) {
	screen_entry_scroll++;
	wmove(screen_entry, 0, screen_entry_offset);
	wdelch(screen_entry);
	wmove(screen_entry, 0, size.ws_col - 2);
	waddch(screen_entry, screen_entry_data[screen_entry_pos - 1]);
	if(screen_entry_pos < screen_entry_len)
		waddch(screen_entry, screen_entry_data[screen_entry_pos]);
}

void screen_entry_scroll_left(void) {
	screen_entry_scroll--;
	screen_entry_update();
}

void screen_entry_scroll_end(void) {
	int len = 0;
	screen_entry_scroll = screen_entry_len;
	while(screen_entry_scroll && len < size.ws_col - (screen_entry_offset + 1)) {
		screen_entry_scroll--;
		len += screen_text_size(screen_entry_data, screen_entry_scroll, 1);
	}
	screen_entry_update();
}

void screen_redraw(void) {
	buffer_t *buffer =  channel_current ? &channel_current->buffer : &server_current->buffer;
	wclear(screen_main);
	if(buffer->wrap)
		screen_addnstr(buffer->data + buffer->pos, BUFFER_SIZE - buffer->pos);
	if(buffer->pos)
		screen_addnstr(buffer->data, buffer->pos - 1);
	wrefresh(screen_main);
	screen_status_update();
	screen_entry_update();
}

void screen_history_add(char *str) {
	if(++screen_history_start == HISTORY_SIZE)
		screen_history_start = 0;
	string_recopy(&screen_history[screen_history_start], str);
	if(screen_history_len < HISTORY_SIZE)
		screen_history_len++;
	screen_history_pos = -1;
}

void screen_entry_set(char *str) {
	int i = 0;
	while((screen_entry_data[i] = str[i]))
		i++;
	screen_entry_len = i;
	screen_entry_pos = i;
	screen_entry_scroll_end();
}

void screen_gotch(int c) {
	int i;
	server_t *s;
	switch(c) {
		case '':
			if(channel_current == NULL && server_current->nchannels)
				channel_focus(server_current, server_current->channels[0]);
			else {
				for(i = 0; i < server_current->nchannels; i++)
					if(server_current->channels[i] == channel_current)
						break;
				if(i < server_current->nchannels - 1)
					channel_focus(server_current, server_current->channels[i + 1]);
				else {
					for(i = 0; i < nservers; i++)
						if(servers[i] == server_current)
							break;
					channel_focus(servers[(i < nservers - 1) ? i + 1 : 0], NULL);
				}
			}
			break;
		case '':
			if(channel_current == NULL) {
				for(i = 0; i < nservers; i++)
					if(servers[i] == server_current)
						break;
				s = servers[i ? i - 1 : (nservers - 1)];
				channel_focus(s, s->nchannels ? s->channels[s->nchannels - 1] : NULL);
			}	else {
				for(i = 0; i < server_current->nchannels; i++)
					if(server_current->channels[i] == channel_current)
						break;
				channel_focus(server_current, (i > 0 && i != server_current->nchannels) ? server_current->channels[i - 1] : NULL);
			}
			break;
		case '':
			server_create(getenv("USER"));
			break;
		case '':
			channel_destroy(server_current, channel_current);
			break;
		case '':
		case KEY_CLEAR:
			screen_redraw();
			break;
		case KEY_UP:
			if(!screen_history_len || (screen_history_pos != -1 && ((screen_history_pos == 0) ?  screen_history_len - 1 : (screen_history_pos - 1)) == screen_history_start))
				break;
			if(screen_history_pos == -1) {
				screen_entry_data[screen_entry_len] = 0;
				screen_entry_copy = string_copy(screen_entry_data);
			}
			screen_history_pos = (screen_history_pos != -1) ? screen_history_pos ?  screen_history_pos - 1 : (screen_history_len - 1) : screen_history_start;
			screen_entry_set(screen_history[screen_history_pos]);
			break;
		case KEY_DOWN:
			if(screen_history_len && screen_history_pos == screen_history_start) {
				screen_history_pos = -1;
				screen_entry_set(screen_entry_copy);
				free(screen_entry_copy);
				screen_entry_copy = NULL;
			}
			if(screen_history_pos == -1)
				break;
			if(++screen_history_pos == screen_history_len)
				screen_history_pos = 0;
			screen_entry_set(screen_history[screen_history_pos]);
			break;
		case KEY_LEFT:
			if(!screen_entry_pos)
				break;
			screen_entry_pos--;
			if(screen_text_size(screen_entry_data, 0, screen_entry_pos) < screen_entry_scroll)
				screen_entry_scroll_left();
			goto move_end;
		case KEY_RIGHT:
			if(screen_entry_pos == screen_entry_len)
				break;
			screen_entry_pos++;
			if(screen_entry_pos_real() >= size.ws_col)
				screen_entry_scroll_right();
			goto move_end;
		case KEY_HOME:
			screen_entry_pos = 0;
			if(screen_entry_scroll) {
				screen_entry_scroll = 0;
				screen_entry_update();
			}
			goto move_end;
		case KEY_END:
			screen_entry_pos = screen_entry_len;
			screen_entry_scroll_end();
			move_end:
			wmove(screen_entry, 0, screen_entry_pos_real());
			wrefresh(screen_entry);
			break;
		case KEY_BACKSPACE:
			if(!screen_entry_pos)
				break;
			screen_entry_pos--;
			wmove(screen_entry, 0, screen_entry_pos_real());
		case KEY_DC:
			if(c == KEY_DC && screen_entry_len - screen_entry_pos == 0)
					break;
			screen_entry_len--;
			for(i = screen_text_size(screen_entry_data, screen_entry_pos, 1); i; i--)
				wdelch(screen_entry);
			wrefresh(screen_entry);
			for(i = screen_entry_pos; i < screen_entry_len; i++)
				screen_entry_data[i] = screen_entry_data[i + 1];
			if(screen_entry_scroll)
				screen_entry_scroll_left();
			break;
		case '\n':
			if(!screen_entry_len)
				break;
			screen_entry_data[screen_entry_len] = 0;
			if(screen_entry_data[0] == '/') {
				if(screen_entry_len > 1)
					if(!command_handle(screen_entry_data + 1))
						dprintf(server_current->socket, "%s\r\n", screen_entry_data + 1);
			} else {
				if(channel_current) {
					dprintf(server_current->socket, "PRIVMSG %s :%s\r\n", channel_current->name, screen_entry_data);
					screen_printf(server_current, channel_current, "<%s> %s\n", server_current->nick, screen_entry_data);
				}
			}
			screen_history_add(screen_entry_data);
			free(screen_entry_copy);
			screen_entry_copy = NULL;
		case '':
		case KEY_DL:
			screen_entry_len = 0;
			screen_entry_pos = 0;
			screen_entry_scroll = 0;
			screen_entry_update();
			break;
		case '':
			exit(0);
		default:
			if(screen_entry_len == sizeof(screen_entry_data))
				break;
			for(i = screen_entry_len; i > screen_entry_pos; i--)
				screen_entry_data[i] = screen_entry_data[i - 1];
			screen_entry_data[screen_entry_pos] = c;
			screen_entry_pos++;
			screen_entry_len++;
			if(screen_entry_pos_real() >= size.ws_col)
				screen_entry_scroll_right();
			else {
				waddch(screen_entry, c);
				waddnstr(screen_entry, screen_entry_data + screen_entry_pos, screen_entry_len - screen_entry_pos);
				wmove(screen_entry, 0, screen_entry_pos_real());
			}
			wrefresh(screen_entry);
	}
}
