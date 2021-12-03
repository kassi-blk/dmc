CC = gcc -m32
CFLAGS = -Wall -Wextra
LDFLAGS = -lncurses -lm
OBJS = main.o
BINNAME = prog

.PHONY: all
all: $(OBJS)
	@$(CC) $(LDFLAGS) -o $(BINNAME) $^

%.o: %.c
	@$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	@rm -f $(BINNAME) *.o
