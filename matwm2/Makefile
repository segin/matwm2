prefix		= /usr/local
manprefix	= /usr/local
XROOT			= /usr/X11R6
CC				= cc

CFLAGS    += -DSHAPE -lXext # comment out this line if you haven't got the x extentions library
CFLAGS		+= -lX11 -I$(XROOT)/include -L$(XROOT)/lib
CFLAGS		+= -DDEBUG -ggdb3 # -DDEBUG_EVENTS -DSYNC

all:	matwm2

matwm2:
		$(CC) $(CFLAGS) *.c -o $@

clean:
		rm -f matwm2

header:
		./makeheader *.c > all.h

install:	matwm2
		mkdir -p $(prefix)/bin $(manprefix)/man/man1
		install -s matwm2 $(prefix)/bin
		install matwm2.1 $(manprefix)/man/man1

deinstall:
		rm -f $(prefix)/bin/matwm2 $(manprefix)/man/man1/matwm2.1* $(manprefix)/man/cat1/matwm2.1*

