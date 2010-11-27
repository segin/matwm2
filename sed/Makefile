# bsd-sed-win32 Makefile
# # # 

#CC 	= 	mingw32-gcc
CC	=	i586-mingw32msvc-gcc 
CFLAGS	+=	-Iregex
LIBS 	+= 	-lregex 
LDFLAGS	+=	$(LIBS) 

PROG	:=	sed
EXESUF	:=	.exe
EXE	:=	$(PROG)$(EXESUF)

SRCS	:=	compile.c main.c misc.c process.c
OBJS	:=	$(SRCS:.c=.o)

all:	$(EXE)

$(EXE):	$(OBJS)	

