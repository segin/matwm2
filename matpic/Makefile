OBJS_MATPIC_AS  = str.o mem.o arch.o pic14b.o as.o main_as.o misc.o

all: matpic_as

matpic_as: $(OBJS_MATPIC_AS)
	$(CC) -o $@ $(OBJS_MATPIC_AS)

clean:
	$(RM) $(OBJS_MATPIC_AS) matpic_as

