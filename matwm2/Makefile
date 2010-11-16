PREFIX    = /usr/local
MANPREFIX = /usr/local
XROOT     = /usr/X11R6
CC        = cc
RM        = rm -f

CFLAGS    += -DSHAPE # comment out this line to disable shape support
LDFLAGS   += -lXext # if shape support is disabled, this normally is no longer necessary and schould probally be commented out too
CFLAGS    += -DDEBUG -ggdb3 # -DDEBUG_EVENTS -DSYNC # for debugging
CFLAGS    += -I$(XROOT)/include
LDFLAGS   += -lX11 -L$(XROOT)/lib
CFLAGS    += -DVERSION=\"0.0.89pre1\"

SRCFILES  := actions.c buttons.c client.c config.c drag.c events.c evn.c ewmh.c info.c input.c main.c misc.c vdesk.c wlist.c x11.c
OBJFILES  := $(SRCFILES:.c=.o)

all: matwm2

matwm2: $(OBJFILES)	
	$(CC) $(LDFLAGS) $(OBJFILES) -o $@

clean:
	$(RM) matwm2 *.o

header:
	./makeheader *.c > all.h

install: matwm2
	mkdir -p $(PREFIX)/bin $(MANPREFIX)/man/man1
	install -s matwm2 $(PREFIX)/bin
	install matwm2.1 $(MANPREFIX)/man/man1

deinstall:
	rm -f $(PREFIX)/bin/matwm2 $(MANPREFIX)/man/man1/matwm2.1* $(MANPREFIX)/man/cat1/matwm2.1*

