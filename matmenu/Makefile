prefix  = /usr/local
XROOT   = /usr/X11R6
CC      = cc

CFLAGS  += -lXext -lX11 -I$(XROOT)/include -L$(XROOT)/lib
CFLAGS += -DDEBUG -ggdb3

all:		matmenu

matmenu:
		$(CC) $(CFLAGS) *.c -o $@

clean:
		rm -f matmenu

header:
		./makeheader *.c > all.h

install:	matmenu
		install -s matmenu $(prefix)/bin

deinstall:
		rm -f $(prefix)/bin/matmenu

