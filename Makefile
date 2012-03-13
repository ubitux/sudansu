CFLAGS += -Wall -Wextra -Wshadow -Wstrict-prototypes -O3
NAME = sudansu
all: $(NAME)
$(NAME): $(NAME).o
clean:
	$(RM) $(NAME).o
distclean: clean
	$(RM) $(NAME) graph*.png
re: distclean all
graphs:
	./graph.py 0 | neato -Tpng > graph0.png && convert graph0.png -resize 500x500 graph0.png
	./graph.py 1 | neato -Tpng > graph1.png && convert graph1.png -resize 500x500 graph1.png
