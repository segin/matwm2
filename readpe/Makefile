#
# readpe Makefile
# 

CC	:= gcc
CFLAGS	:= -m32
OBJEXT	:= .o
BINEXT	:= .exe
PROG	:= readpe


SOURCES	:= readpe.c
OBJS	:= $(SOURCES:.c=$(OBJEXT))

all: $(PROG)$(BINEXT)

$(PROG)$(BINEXT): $(OBJS)
