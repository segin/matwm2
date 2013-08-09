CC			:= gcc
LD			:= gcc
CFLAGS	:= `pkg-config --cflags sdl SDL_gfx`
LDFLAGS	:= `pkg-config --libs sdl SDL_gfx`
OBJS		:= bezier.o
PROG		:= bezier
RM			:= rm -f 

$(PROG): $(OBJS)

clean: 
	$(RM) $(PROG) $(OBJS) *~