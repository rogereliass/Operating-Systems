# CC = gcc
# CFLAGS = -Wall -g -Iinclude
# SRCS = src/main.c src/os.c src/memory.c src/parser.c src/scheduler.c src/semaphore.c
# OBJS = $(SRCS:.c=.o)
# BIN  = os_sim

# all: $(BIN)

# $(BIN): $(OBJS)
# 	$(CC) $(CFLAGS) -o $@ $^

# clean:
# 	rm -f $(OBJS) $(BIN)

# CC = gcc
# CFLAGS = -Wall -g
# INCLUDES = -Iinclude

SRC = src/main.c src/os.c src/memory.c src/semaphore.c src/parser.c \
      src/priority_queue.c src/fcfs_scheduler.c src/round_robin_scheduler.c \
      src/mlfq_scheduler.c
OBJ = $(SRC:.c=.o)

TARGET = os_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)