CC       = cc
RM       = rm -f

OBJFILES = tcp.o irc.o misc.o main.o screen.o commands.o

CFLAGS   = -ggdb3
LIBS     = -lcurses

all: matirc

matirc: $(OBJFILES)
	$(CC) -o $@ $(OBJFILES) $(LIBS)

clean:
	$(RM) matirc *.o
