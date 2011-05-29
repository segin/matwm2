#
# mcres Makefile
# 

CC	:= gcc
CFLAGS	:= -m32 
LDFLAGS	:= -mwindows
OBJEXT	:= .o
BINEXT	:= .exe
PROG	:= mcres
WRC		:= windres

RC	:= mcres.rc
SOURCES	:= mcres.c 
OBJS	:= $(SOURCES:.c=$(OBJEXT)) $(RC:.rc=-rc$(OBJEXT)) 

all: $(PROG)$(BINEXT)

mcres-rc.o: $(RC)
	windres -F pe-i386 mcres.rc -o mcres-rc.o

$(PROG)$(BINEXT): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(PROG)$(BINEXT)