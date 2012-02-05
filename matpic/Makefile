CC = wcc386
LD = wlink
RM = del

OBJS_MATPIC_AS  = str.obj mem.obj arch.obj pic14b.obj as.obj main_as.obj

all: matpic_as.exe

.c.obj : .autodepend
	$(CC) $(CFLAGS) $<

matpic_as.exe: $(OBJS_MATPIC_AS)
	$(LD) name $@ file { $< }

clean: .symbolic
	$(RM) $(OBJS_MATPIC_AS) matpic_as.exe
