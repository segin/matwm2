# Makefile for syscalls

SRC = exit.c fork.c write.c
OBJ = $(SRC:.c=.o) syscall.o

CFLAGS = -nostdinc -I../include 

all: libsys.a

libsys.a: $(OBJ)
	ar rcs $@ $(OBJ)

syscall.o: 
	as syscall.s -o $@

clean:
	rm -f libsys.a
	rm -f $(OBJ)
