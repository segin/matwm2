PREFIX  = /usr/local

CC      = cc
RM      = rm -f
INSTALL = install

CFLAGS  += -g -Wall -std=c99 -pedantic
CFLAGS  += -fno-strict-overflow # stop GCC making silly assumption

OBJS    = str.o mem.o arch.o pic14b.o pic18f.o as.o main.o misc.o ihex.o dis.o ppc.o io.o lineno.o

all: matpic

matpic: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	$(RM) $(OBJS) matpic

install: matpic
	$(INSTALL) -s matpic $(PREFIX)/bin/matpic

deinstall:
	$(RM) $(PREFIX)/bin/matpic
