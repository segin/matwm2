
PREFIX  = /usr/local

RM      = rm -f
INSTALL = install

OBJS = closure.o graph.o lr0.o mkpar.o reader.o symtab.o warshall.o error.o lalr.o main.o output.o skeleton.o verbose.o

all: matyacc

matyacc: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	$(RM) $(OBJS) matyacc

install: matyacc
	$(INSTALL) matyacc $(PREFIX)/bin
	$(INSTALL) matyacc.1 $(PREFIX)/man/man1
