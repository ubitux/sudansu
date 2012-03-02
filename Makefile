CFLAGS += -Wall -Wextra -Wshadow -Wstrict-prototypes -O3
NAME = sudansu
all: $(NAME)
$(NAME): $(NAME).o
clean:
	$(RM) $(NAME).o
distclean: clean
	$(RM) $(NAME)
re: distclean all
