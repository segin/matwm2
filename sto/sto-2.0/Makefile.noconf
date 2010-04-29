#
# Makefile for sto. Written by segin <segin2005@gmail.com>
#
CC=gcc
CFLAGS=-fno-ident -fomit-frame-pointer -Wl,--strip-all -pedantic -ansi -Os\
	-Dunix
#CC=cc
#CFLAGS=-D_MINIX -D_POSIX_SOURCE
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/man
WINCROSSCC=i386-mingw32msvc-gcc
WINCROSSCFLAGS=-fno-ident -fomit-frame-pointer -Wl,--strip-all -pedantic -ansi\
	-Os
WINCC=dmc.exe
WINCFLAGS=-DDIGITAL_MARS_C -D__WIN32__
DOSCC=tcc
DOSCFLAGS=-O -f87 -1 -Z -mt
ELKSCC=bcc
ELKSCFLAGS=-ansi -O -Dunix
RM=rm -f

# Here we assume a UNIX target.
default all sto:   	
	$(CC) $(CFLAGS) sto.c -o sto 

# Clean the files!
clean:
	$(RM) *.exe sto-elks sto

# UNIX only.
install:
	install -c -m 755 sto $(BINDIR)
	install -c -m 644 sto.1 $(MANDIR)/man1

# Assuming that you're using the xmingw package from Gentoo's portage
# You can change this if you need to.
wincross:
	$(WINCROSSCC) $(WINCROSSCFLAGS) sto.c -o sto32.exe

# No need to specify the source file extension or output -- 
# Turbo C known that a specified source file of just 'sto' means to
# compile file sto.c to sto.obj, and link sto.obj to sto.exe, generating
# sto.map as a link-map
dossto:
	$(DOSCC) $(DOSCFLAGS) sto

#
# Assuming you are using the Digital Mars C/C++ compiler.
# This compiler produces programs that run on Win32s.
#
win:
	$(WINCC) $(WINCFLAGS) sto.c -o sto-reloc.exe

#
# STO also works on ELKS. ELKS is also a UNIX-like OS, so  
# use -Dunix
#
elks:
	$(ELKSCC) $(ELKSCFLAGS) sto.c -o sto-elks 
