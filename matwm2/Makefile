XROOT = /usr/X11R6
all:
	perl makeheader
	cc -lX11 -I$(XROOT)/include -L$(XROOT)/lib *.c -o matwm2

