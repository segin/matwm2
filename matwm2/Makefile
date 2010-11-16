CC = cc
XROOT = /usr/X11R6
CFLAGS = -lXext -lX11 -I$(XROOT)/include -L$(XROOT)/lib
CFLAGS += -DDEBUG

all:
	perl makeheader
	$(CC) $(CFLAGS) *.c -o matwm2

