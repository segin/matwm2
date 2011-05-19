#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "main.h"

#include <curses.h>
#include <stdarg.h>
#include <signal.h>

#ifdef __WIN32__
#include <windows.h>

struct winsize
{
  unsigned short ws_row;	/* rows, in characters */
  unsigned short ws_col;	/* columns, in characters */
  unsigned short ws_xpixel;	/* horizontal size, pixels */
  unsigned short ws_ypixel;	/* vertical size, pixels */
};

void screen_get_win32_console_size(struct winsize *ws);
#endif

#define ENTRY_SIZE 1024
#define HISTORY_SIZE 512

extern WINDOW *screen_main, *screen_entry;
#ifdef SIGWINCH
extern bool screen_resized;
#endif

void screen_initialize(void);
#ifdef SIGWINCH
void screen_update_size(void);
#endif
void screen_end(void);
void screen_printf(server_t *s, channel_t *c, char *format, ...);
void screen_status_update(void);
void screen_redraw(void);
void screen_gotch(int c);

#endif /* __SCREEN_H__ */
