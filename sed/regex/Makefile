# libregex: BSD regex library
# forked from FreeBSD 7.2-RELEASE-p3

#CC	=	mingw32-gcc
#CC	=	i586-mingw32msvc-gcc
CC	=	gcc -m32
CFLAGS	+=	-I. -Igrot


SRCS	=	regcomp.c regerror.c regexec.c regfree.c 
OBJS	:=	$(SRCS:.c=.o)

all: libregex.a

libregex.a: $(OBJS)
