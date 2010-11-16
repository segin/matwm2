PREFIX    = /usr/local
MANPREFIX = /usr/local
XROOT     = /usr/X11R6
CC        = cc
RM        = rm -f

CFLAGS    += -DSHAPE # comment out this line to disable shape support
LDFLAGS   += -lXext # if shape support is disabled, this normally is no longer necessary and schould probally be commented out too
CFLAGS    += -I$(XROOT)/include
LDFLAGS   += -lX11 -L$(XROOT)/lib
#CFLAGS    += -DDEBUG -ggdb3 # -DDEBUG_EVENTS -DSYNC # for debugging
#CFLAGS    += -pedantic -std=c89 -Wall -W -Wno-unused-parameter
CFLAGS    += -DVERSION=\"0.0.92\"

SRCFILES  = actions.c buttons.c client.c config.c defcfg.c drag.c events.c evn.c ewmh.c info.c input.c main.c misc.c vdesk.c wlist.c x11.c
OBJFILES  = $(SRCFILES:.c=.o)
HEADERS   = all.h defcfg.h ewmh.h matwm.h mwm_hints.h

all: matwm2

matwm2: $(OBJFILES) $(HEADERS)
	$(CC) $(LDFLAGS) $(OBJFILES) -o $@

clean:
	$(RM) matwm2 *.o

defcfg:
	perl makedefcfg.pl

header:
	perl makeheader.pl

install: matwm2
	mkdir -p $(PREFIX)/bin $(MANPREFIX)/man/man1
	install -s matwm2 $(PREFIX)/bin
	install matwm2.1 $(MANPREFIX)/man/man1

deinstall:
	$(RM) $(PREFIX)/bin/matwm2 $(MANPREFIX)/man/man1/matwm2.1* $(MANPREFIX)/man/cat1/matwm2.1*
