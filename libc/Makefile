# Makefile for libc

all: libc.a

libc.a: sys/libsys.a
	ar rcs $^

sys/libsys.a:
	cd sys && make

clean:
	cd sys && make clean
	rm -f libc.a
