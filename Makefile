CFLAGS = -std=c99 -Wextra -Wall -Werror
EXEC = rain
LINKS = -lncurses

all: $(EXEC)

$(EXEC): rain.c
	$(CC) $(CFLAGS) *.c -o $(EXEC) $(LINKS)

clean:
	@$(RM) $(EXEC)

