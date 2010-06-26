#
# Makefile for NetMSG-C
#

CC=gcc
CFLAGS=-march=pentium4 -DWANT_PAINTED_GUI -ggdb3
LDFLAGS=-lcurses -lpthread

all: netmsg
	
netmsg: netmsg.c
	$(CC) $(CFLAGS) $(LDFLAGS) netmsg.c -o netmsg

netmsg-snprintf: netmsg.c snprintf.c
	$(CC) $(CFLAGS) -DNEED_ASPRINTF -DNEED_VASPRINTF $(LDFLAGS) snprintf.c \
		netmsg.c -o netmsg

	
clean:
	rm -f *.o *~ netmsg core *.core 
