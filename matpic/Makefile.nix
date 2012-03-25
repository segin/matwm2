PREFIX  = /usr/local

CC      = cc
RM      = rm -f
INSTALL = install

CFLAGS  += -g -Wall -std=c99 -pedantic

OBJS    = str.o mem.o arch.o pic14b.o as.o main.o misc.o ihex.o dis.o ppc.o io.o lineno.o

all: matpic

matpic: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	$(RM) $(OBJS) matpic

install: matpic
	$(INSTALL) -s matpic $(PREFIX)/bin/matpic

deinstall:
	$(RM) $(PREFIX)/bin/matpic
