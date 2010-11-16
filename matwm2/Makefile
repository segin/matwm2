XROOT = /usr/X11R6
CC = gcc
#could put something here in the future
DEFINES = 
CFLAGS = -I$(XROOT)/include $(DEFINES)
LDFLAGS = -lX11 -L$(XROOT)/lib 

all: 
	./makeheader
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -o matwm2

clean:
	rm -f *.o *~ matwm2 core *.core matwm.h
