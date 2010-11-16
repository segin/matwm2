prefix  = /usr/local
XROOT   = /usr/X11R6
CC      = cc

CFLAGS  += -lXext -lX11 -I$(XROOT)/include -L$(XROOT)/lib
#DEFINES += -DDEBUG

all:		matwm2

matwm2:
		$(CC) $(DEFINES) $(CFLAGS) *.c -o $@

clean:
		rm -f matwm2

header:
		./makeheader

install:	matwm2
		install -s matwm2 $(prefix)/bin

deinstall:
		rm -f $(prefix)/bin/matwm2

