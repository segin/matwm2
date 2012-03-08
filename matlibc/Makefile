ARCH = amd64/linux

#CC = cc
#AS = as
RM = rm -f

AFLAGS  = -nostdinc
CFLAGS  = -Wall -nostdinc -fno-builtin -I./include
LDFLAGS = -nostartfiles

include arch/Makefile
include string/Makefile
include stdlib/Makefile

all: test

test: $(OBJS) $(CRTO) test.o
	$(LD) $(LDFLAGS) $(OBJS) $(CRTO) test.o -o $@

clean:
	$(RM) $(OBJS) $(CRTO) test.o test

.SUFFIXES: .S .c .o

.S.o:
	$(AS) $(AFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@
