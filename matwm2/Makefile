CC      = cc
XROOT   = /usr/X11R6

CFLAGS  += -lXext -lX11 -I$(XROOT)/include -L$(XROOT)/lib
DEFINES = -DDEBUG
SRCS    = client.c events.c main.c x11.c config.c icons.c misc.c drag.c input.c wlist.c xev.c
OBJS    = $(SRCS:.c=.o)

all:	matwm2

matwm2: $(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) $(OBJS) -o $@

clean:
	rm -f *.o
	rm -f matwm2

