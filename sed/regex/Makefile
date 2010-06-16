# libregex: BSD regex library
# forked from FreeBSD 7.2-RELEASE-p3

CC	=	mingw32-gcc
CFLAGS	+=	-I. -Igrot


SRCS	=	engine.c regcomp.c regerror.c regexec.c regfree.c 
OBJS	:=	$(SRCS:.c=.o)

all: libregex.a

libregex.a: $(OBJS)
