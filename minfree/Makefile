PREFIX = /usr/bin
NAME   = minfree

all: $(NAME)

$(NAME): $(NAME).o

install:
	install minfree $(PREFIX)

clean:
	rm -f $(NAME) $(NAME).o
