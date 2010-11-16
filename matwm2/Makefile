PREFIX    = /usr/local
MANPREFIX = /usr/local
XROOT     = /usr/X11R6
CC        = cc
RM        = rm -f

CFLAGS    += -I$(XROOT)/include # remove the + for this and the next line if make complains about being unable to add to a non-existing macro
LDFLAGS   += -lX11 -L$(XROOT)/lib
CFLAGS    += -DSHAPE # comment out this line to disable shape support
LDFLAGS   += -lXext # if shape support is disabled, this normally is no longer necessary and schould probally be commented out too
#CFLAGS    += -DNO_VFORK # uncomment this out to compile on minix or other operating systems that lack vfork()

#CFLAGS    += -DDEBUG -ggdb3 # -DDEBUG_EVENTS -DSYNC # for debugging
#CFLAGS    += -pedantic -std=c89 -W -Wall -Wno-unused-parameter
CFLAGS    += -DVERSION=\"0.0.93\"

OBJFILES  = actions.o buttons.o client.o config.o defcfg.o drag.o events.o evn.o ewmh.o info.o input.o main.o misc.o vdesk.o wlist.o x11.o
HEADERS   = all.h defcfg.h ewmh.h matwm.h mwm_hints.h

all: matwm2

matwm2: $(OBJFILES) $(HEADERS)
	$(CC) -o $@ $(OBJFILES) $(LDFLAGS)

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

