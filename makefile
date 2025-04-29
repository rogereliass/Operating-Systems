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

SRC = main.c os.c memory.c semaphore.c parser.c priority_queue.c \
      fcfs_scheduler.c round_robin_scheduler.c mlfq_scheduler.c

OBJ = $(SRC:.c=.o)

TARGET = os_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)