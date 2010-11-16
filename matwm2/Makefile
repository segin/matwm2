prefix  = /usr/local
XROOT   = /usr/X11R6
CC      = cc

CFLAGS  += -lXext -lX11 -I$(XROOT)/include -L$(XROOT)/lib
#CFLAGS  += -DDEBUG -ggdb3 # -DDEBUG_EVENTS -DSYNC

all:		matwm2

matwm2:
		$(CC) $(CFLAGS) *.c -o $@

clean:
		rm -f matwm2

header:
		./makeheader *.c > all.h

install:	matwm2
		install -s matwm2 $(prefix)/bin

deinstall:
		rm -f $(prefix)/bin/matwm2

