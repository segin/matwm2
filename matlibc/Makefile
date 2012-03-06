ARCH = amd64/linux

AFLAGS  = -nostdinc
CFLAGS  = -nostdinc -fno-builtin -I./include
LDFLAGS = -nostartfiles

all: test

DIR = arch
include $(DIR)/Makefile
DIR = string
include $(DIR)/Makefile

test: $(OBJS) string arch matcrt0.o test.o string
	$(LD) $(LDFLAGS) $(OBJS) matcrt0.o test.o -o $@

clean:
	$(RM) $(OBJS) matcrt0.o test.o test

.SUFFIXES: .S .c .o

.S.o:
	$(AS) $(AFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@
