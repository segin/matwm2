OBJS = str.o mem.o arch.o pic14b.o as.o main.o misc.o ihex.o dis.o vstr.o

all: matpic

matpic: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	$(RM) $(OBJS) matpic

