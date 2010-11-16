PREFIX    = /usr/local
MANPREFIX = /usr/local
XROOT     = /usr/X11R6
CC        = cc
RM        = rm -f

CFLAGS    += -I$(XROOT)/include # remove the + for this and the next two lines if make complains about being unable to add to a non-existing macro
LDFLAGS   += -L$(XROOT)/lib
LIBS      += -lX11
CFLAGS    += -DSHAPE # comment out this line to disable shape support
LIBS      += -lXext # if shape support is disabled, this normally is no longer necessary and schould probally be commented out too
CFLAGS    += -DXFT -I$(PREFIX)/include/freetype2 # same as above two but for Xft font support
LIBS      += -lXft

#CFLAGS    += -DDEBUG -ggdb3 # -DDEBUG_EVENTS -DSYNC # for debugging
CFLAGS    += -Wall -Wextra -Wno-unused-parameter
CFLAGS    += -DVERSION=\"0.0.97\"

OBJFILES  = actions.o buttons.o client.o config.o defcfg.o drag.o events.o evn.o ewmh.o info.o input.o main.o misc.o vdesk.o wlist.o x11.o
HEADERS   = all.h defcfg.h ewmh.h matwm.h mwm_hints.h

all: matwm2

matwm2: $(OBJFILES) $(HEADERS)
	$(CC) $(LDFLAGS) -o $@ $(OBJFILES) $(LIBS)

clean:
	$(RM) matwm2 *.o

# converts default_matwmrc into C code that can be compiled into the binary
defcfg:
	perl makedefcfg.pl

# generates all.h wich defines most global variables and functions
header:
	perl makeheader.pl

install: matwm2
	mkdir -p $(PREFIX)/bin $(MANPREFIX)/man/man1
	install -s matwm2 $(PREFIX)/bin
	install matwm2.1 $(MANPREFIX)/man/man1

deinstall:
	$(RM) $(PREFIX)/bin/matwm2 $(MANPREFIX)/man/man1/matwm2.1* $(MANPREFIX)/man/cat1/matwm2.1*

