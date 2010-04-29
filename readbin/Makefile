
CC	=	gcc
DEFINES	=	-D__ELF__ -D__NO_AOUT__ -D__PE__
CFLAGS 	=	$(DEFINES) -Wall -pedantic --std=c99
CS	=	mcs

default: all

all:	readbin

clean:
	rm -f a.out readbin readbin.exe *~ core *.core

dotnet:
	# Assuming Mono, kthx
	$(CS) readbin.cs
