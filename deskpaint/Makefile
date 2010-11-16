prefix  = /usr/local
XROOT   = /usr/X11R6
CC      = cc

CFLAGS  += -lXext -lX11 -I$(XROOT)/include -L$(XROOT)/lib

all:
		$(CC) $(CFLAGS) *.c -o deskpaint

