OBJS = closure.o graph.o lr0.o mkpar.o reader.o symtab.o warshall.o error.o lalr.o main.o output.o skeleton.o verbose.o

all: matyacc

matyacc: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	$(RM) $(OBJS) matyacc

