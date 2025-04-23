CC = gcc
CFLAGS = -Wall -g -Iinclude
SRCS = src/main.c src/os.c src/memory.c src/parser.c src/scheduler.c src/semaphore.c
OBJS = $(SRCS:.c=.o)
BIN  = os_sim

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(BIN)
