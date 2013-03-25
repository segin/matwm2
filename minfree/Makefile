PREFIX = /usr/bin
NAME   = minfree
CFLAGS += -O2

all: $(NAME)

$(NAME): $(NAME).o

install:
	install minfree $(PREFIX)

clean:
	rm -f $(NAME) $(NAME).o
