OBJS = str.o mem.o arch.o pic14b.o as.o main.o misc.o ihex.o dis.o ppc.o host.o

all: matpic

matpic: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	$(RM) $(OBJS) matpic

